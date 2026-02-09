#include "IotaWatt.h"

/***************************************************************************************************
 *  GetFeedData SERVICE.
 *  
 *  The web server does a pretty good job of handling file downloads and uploads asynchronously, 
 *  but assumes that any callbacks to new handlers get the job completely done befor returning. 
 *  The GET /feed/data/ request takes a long time and generates a lot of data so it needs to 
 *  run as a SERVICE so that sampling can continue while it works on providing the data.  
 *  To accomplish that without modifying ESP8266WebServer, we schedule this SERVICE and
 *  block subsequent calls to server.handleClient() until the request is satisfied, at which time
 *  this SERVICE returns with code 0 to cause it's serviceBlock to be deleted.  When a new /feed/data
 *  request comes in, the web server handler will reshedule this SERVICE with NewService.
 * 
 *  The process time per dispatch is determined to try to balance sampling with response time.
 *  When a new request is soon after another, the process time is decreased to maintain
 *  sampling at the expense of response time.
 * 
 *  ********NOTE*******
 *  7/22/2018 Made this a direct call from webserver so entire transaction is handled without
 *  returning to allow sampling. This seems to eliminate memory leak problems with sending the
 *  response data after essentially ending the transaction by returning from the initial webserver call.
 *  Improves query response times, but stops sampling for about 1.3-1.5sec for a typical graph request.
 * 
 *  Stopping sampling doesn't mean power is not logged.  It only means that changes to power
 *  during that period will not be measured. 
 * 
 *  Other solutions that can be explored in the future might be:
 * 
 *  1) sample power directly from within the webserver handler (ugh).
 *  2) determine what is not being cleaned up and find a way to do so. Suspect it's 
 *     request headers as the problem is more severe when digest auth headers are collected.
 *  3) Try using asyncwebserver.
 *   
 **************************************************************************************************/

uint32_t getFeedData(){ //(struct serviceBlock* _serviceBlock){
  // trace T_GFD

  struct req {
    req* next;
    int channel;
    char queryType;
    Script* output;
    req(){next=nullptr; channel=0; queryType=' '; output=nullptr;};
    ~req(){delete next;};
  }; 

  static IotaLogRecord* logRecord = nullptr;
  static IotaLogRecord* lastRecord = nullptr;
  static size_t   chunkSize = 1600;
  static char* buf = nullptr;
  static size_t bufPos = 0;
  static char*    rowBuf = nullptr;
  static req*     reqRoot = nullptr;
  static uint32_t startUnixTime;
  static uint32_t endUnixTime;
  static uint32_t intervalSeconds;
  static uint32_t UnixTime;
  static uint32_t lastReqTime = 0;
  static uint32_t processInterval;
  static uint32_t startTime;
  static bool     modeRequest;
  enum   states   {setup, process} static state = setup;
       
  switch (state) {
    
    case setup: {
      trace(T_GFD,0);
      startTime = millis();
      processInterval = 3000 / (uint32_t)(frequency + 0.1);
      if((millis() - lastReqTime) > 10000){
        processInterval = 6000 / (uint32_t)(frequency + 0.1);
      }
      lastReqTime = millis();

        // Validate the request parameters
      
      startUnixTime = server.arg("start").substring(0,10).toInt();
      endUnixTime = server.arg("end").substring(0,10).toInt();
      if(server.hasArg("interval")){
        intervalSeconds = server.arg("interval").toInt();
        modeRequest = false;;
      }
      else if(server.hasArg("mode")){
        modeRequest = true;
        if(server.arg("mode")== "daily") intervalSeconds = 86400;
        else if(server.arg("mode") == "weekly") intervalSeconds = 86400 * 7;
        else if(server.arg("mode") == "monthly") intervalSeconds = 86400 * 30;
        else if(server.arg("mode") == "yearly") intervalSeconds = 86400 * 365;
      }
      if((startUnixTime % 5) ||
         (endUnixTime % 5) ||
         (intervalSeconds % 5) ||
         (intervalSeconds <= 0) ||
         (endUnixTime < startUnixTime) ||
         ((endUnixTime - startUnixTime) / intervalSeconds > 2000)) {
        server.send(400, "text/plain", "Invalid request");
        state = setup;
        serverAvailable = true;
        return 0;    
      }
      
          // Parse the ID parm into a list.
      
      String idParm = server.arg("id");
      if( ! reqRoot) reqRoot = new req;
      req* reqPtr = reqRoot;
      int i = 0;
      if(idParm.startsWith("[")){
        idParm[idParm.length()-1] = ',';
        i = 1;
      } else {
        idParm += ",";
      }
      while(i < idParm.length()){
        reqPtr->next = new req;
        reqPtr = reqPtr->next;
        String id = idParm.substring(i,idParm.indexOf(',',i));
        String name = id.substring(2);
        i = idParm.indexOf(',',i) + 1;
        if(id.charAt(0) == 'I'){
          for(int j=0; j<maxInputs; j++){
            if(inputChannel[j]->isActive() &&
               name.equals(inputChannel[j]->_name)){
               reqPtr->channel = inputChannel[j]->_channel;
               reqPtr->output = nullptr;
               reqPtr->queryType = id.charAt(1);
               break;
            }
          }
        }
        else if(id.charAt(0) == 'O'){
          Script* script = outputs->first();
          while(script){
            if(name.equals(script->name())){
              reqPtr->channel = -1;
              reqPtr->output = script;
              reqPtr->queryType = id.charAt(1);
              break;
            } 
            script = script->next(); 
          }  
        }
      }
          
      logRecord = new IotaLogRecord;
      lastRecord = new IotaLogRecord;
     
      if(startUnixTime >= History_log.firstKey()){   
        lastRecord->UNIXtime = startUnixTime - intervalSeconds;
      } else {
        lastRecord->UNIXtime = History_log.firstKey();
      }
      logReadKey(lastRecord);
      
          // Using String for a large buffer abuses the heap
          // and takes up a lot of time. We will build 
          // relatively short response elements with String
          // and copy them to this larger buffer.

      buf = new char[chunkSize+8];
      bufPos = 6;
      rowBuf = new char[chunkSize+8];
      
          // Setup buffer to do it "chunky-style"
      
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200,"application/octet-stream","");
      strcpy(rowBuf, "[");
      UnixTime = startUnixTime;
      state = process;
    }
  
    case process: {
      trace(T_GFD,1);

          // Loop to generate entries
      
      while(UnixTime <= endUnixTime) {
        int rtc;
        logRecord->UNIXtime = UnixTime;
        logReadKey(logRecord);
        trace(T_GFD,2);

        char* p = rowBuf + strlen(rowBuf);
        *p++ = '[';

        double elapsedHours = logRecord->logHours - lastRecord->logHours;
        req* reqPtr = reqRoot;
        while((reqPtr = reqPtr->next) != nullptr){
          int channel = reqPtr->channel;
          int remaining = (rowBuf + chunkSize + 8) - p;

          // Safety check for buffer space
          if (remaining <= 0) remaining = 0;

          if(rtc || logRecord->logHours == lastRecord->logHours){
            if (remaining > 0) {
                int len = snprintf(p, remaining, "null");
                if (len > 0) p += MIN(len, remaining - 1);
            }
          }
  
            // input channel

          else if(channel >= 0){
            trace(T_GFD,3);
            double val = 0;
            int precision = 1;
            bool isNull = false;

            if(reqPtr->queryType == 'V') {
              val = (logRecord->accum1[channel] - lastRecord->accum1[channel]) / elapsedHours;
              precision = 1;
            } 
            else if(reqPtr->queryType == 'P') {
              val = (logRecord->accum1[channel] - lastRecord->accum1[channel]) / elapsedHours;
              precision = 1;
            }
            else if(reqPtr->queryType == 'E') {
                val = (logRecord->accum1[channel] / 1000.0);
                precision = 3;
            } 
            else {
              isNull = true;
            }

            if (remaining > 0) {
                int len = 0;
                if (isNull || isnan(val) || isinf(val)) {
                     len = snprintf(p, remaining, "null");
                } else {
                     len = snprintf(p, remaining, "%.*f", precision, val);
                }
                if (len > 0) p += MIN(len, remaining - 1);
            }
          }
  
           // output channel
          
          else {
            trace(T_GFD,4);
            if (remaining > 0) {
                int len = 0;
                if(reqPtr->output == nullptr){
                  len = snprintf(p, remaining, "null");
                }
                else {
                    double val = 0;
                    int precision = 1;

                    if(reqPtr->queryType == 'V'){
                      val = reqPtr->output->run(lastRecord, logRecord, Volts);
                      precision = 1;
                    }
                    else if(reqPtr->queryType == 'P'){
                      val = reqPtr->output->run(lastRecord, logRecord, Watts);
                      precision = 1;
                    }
                    else if(reqPtr->queryType == 'E'){
                        val = reqPtr->output->run(nullptr, logRecord, kWh);
                        precision = 3;
                    }
                    else if(reqPtr->queryType == 'O'){
                      val = reqPtr->output->run(lastRecord, logRecord);
                      precision = reqPtr->output->precision();
                    }
                    else {
                      val = NAN; // force null
                    }

                    if (isnan(val) || isinf(val)) {
                        len = snprintf(p, remaining, "null");
                    } else {
                        len = snprintf(p, remaining, "%.*f", precision, val);
                    }
                }
                if (len > 0) p += MIN(len, remaining - 1);
            }
          }

          // Add comma
          if ((rowBuf + chunkSize) - p > 1) {
              *p++ = ',';
              *p = '\0';
          }
        } 
           
        // Replace last comma with ]
        if (*(p-1) == ',') {
            *(p-1) = ']';
        } else {
             // Should not happen if loop ran, but if empty loop:
             *p++ = ']';
             *p = '\0';
        }

        IotaLogRecord* swapRecord = lastRecord;
        lastRecord = logRecord;
        logRecord = swapRecord;
        UnixTime += intervalSeconds;

            // If not enough room in buffer for this segment, 
            // Write the buffer chunk.

        size_t rowLen = p - rowBuf;

        if((bufPos + rowLen) > (chunkSize - 3)){
          sendChunk(buf, bufPos);
          bufPos = 6;
        }    

            // Add this segment to buf.

        trace(T_GFD,5);
        memcpy(buf + bufPos, rowBuf, rowLen);
        bufPos += rowLen;
        
        // Prepare for next iteration
        strcpy(rowBuf, ",");
      }
      trace(T_GFD,7);

          // All entries generated, terminate Json and send.
      
      // Close the array
      // replyData->setCharAt(replyData->length()-1,']');
      // Here rowBuf has "," from end of loop. We need to replace comma with ].
      if (rowBuf[0] == ',') {
          rowBuf[0] = ']';
          rowBuf[1] = '\0';
      } else {
          // Can happen if loop never ran
          strcat(rowBuf, "]");
      }

      size_t finalLen = strlen(rowBuf);
      memcpy(buf + bufPos, rowBuf, finalLen);
      bufPos += finalLen;
      sendChunk(buf, bufPos);

      delete[] rowBuf;
      rowBuf = nullptr;
      
          // Send terminating zero chunk, clean up and exit.    
      
      sendChunk(buf, 6);
      server.client().stop();
      trace(T_GFD,7);
      delete[] buf;
      buf = nullptr;
      delete reqRoot;
      reqRoot = nullptr;
      delete logRecord;
      logRecord = nullptr;
      delete lastRecord;
      lastRecord = nullptr;
      state = setup;
      serverAvailable = true;
      return 0;                                       // Done for now, return without scheduling.
    }
  }
  return 0;
}