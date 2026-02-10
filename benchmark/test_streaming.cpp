#include <iostream>
#include <string>
#include <vector>

// Mock Objects to simulate Arduino environment
struct MockDir {
    std::vector<std::string> files;
    std::vector<std::string> dirs;
    int index_dirs = -1;
    int index_files = -1;
    bool is_dir = false;

    bool next() {
        if (index_dirs < (int)dirs.size()) {
            if (index_dirs == -1 && !dirs.empty()) {
                index_dirs = 0;
                is_dir = true;
                return true;
            }
            if (index_dirs + 1 < (int)dirs.size()) {
                index_dirs++;
                is_dir = true;
                return true;
            }
            index_dirs = dirs.size(); // Finished dirs
        }

        // If we are here, we are checking files
        if (index_files < (int)files.size()) {
            if (index_files == -1 && !files.empty()) {
                index_files = 0;
                is_dir = false;
                return true;
            }
            if (index_files + 1 < (int)files.size()) {
                index_files++;
                is_dir = false;
                return true;
            }
            index_files = files.size();
        }
        return false;
    }

    // Simpler mock implementation matching typical Dir usage
    // Usually one loop iterates everything.
    // But the original code rewinds and loops twice!
    // So MockDir needs to support rewind and sequential access.

    // Let's implement a single vector of entries
    struct Entry {
        std::string name;
        bool isDir;
    };
    std::vector<Entry> entries;
    int current = -1;

    void addDir(std::string name) { entries.push_back({name, true}); }
    void addFile(std::string name) { entries.push_back({name, false}); }

    void rewind() { current = -1; }

    bool next_entry() {
        if (current + 1 < (int)entries.size()) {
            current++;
            return true;
        }
        return false;
    }

    bool isDirectory() { return entries[current].isDir; }
    bool isFile() { return !entries[current].isDir; }
    std::string fileName() { return entries[current].name; }

    // Simulate ignore case
    bool equalsIgnoreCase(const std::string& other) {
        // Simple case insensitive compare for mock
        // system volume information
        std::string a = fileName();
        std::string b = other;
        // simplistic
        return a == b;
    }
};

struct MockServer {
    std::string content;
    bool chunked = false;

    void setContentLength(int len) {
        if (len == -1) chunked = true; // CONTENT_LENGTH_UNKNOWN
    }

    void send(int code, const char* type, const std::string& msg) {
        // Headers sent here
        std::cout << "Server send: " << code << " " << type << " " << msg << std::endl;
        content += msg;
    }

    void sendContent(const std::string& msg) {
        //std::cout << "Server chunk: " << msg << std::endl;
        content += msg;
    }

    bool hasArg(const std::string& arg) { return true; }
    std::string arg(const std::string& arg) { return "/"; }
};

MockServer server;
MockDir mockDir;

struct MockSDFS {
    MockDir openDir(const std::string& path) {
        return mockDir;
    }
} SDFS;

struct MockSD {
    bool exists(const char* path) { return true; }
} SD;

const char appJson_P[] = "application/json";
#define F(x) x
#define PROGMEM

// The function to test (the optimized version)
void printDirectory_Streaming() {
  std::string path = "/"; // server.arg("dir");

  if(path != "/" && !SD.exists(path.c_str())) return; // returnFail("BAD PATH");

  MockDir dir = SDFS.openDir(path);

  server.setContentLength(-1); // CONTENT_LENGTH_UNKNOWN
  server.send(200, appJson_P, "");

  server.sendContent("[");

  bool first = true;
  if(path == "/"){
      server.sendContent("{\"type\":\"dir\",\"name\":\"esp_spiffs\"}");
      first = false;
  }

  dir.rewind();
  while(dir.next_entry()){
      if(dir.isDirectory() && dir.fileName() != "system volume information"){
          if(!first) server.sendContent(",");
          server.sendContent("{\"type\":\"dir\",\"name\":\"" + dir.fileName() + "\"}");
          first = false;
      }
  }

  dir.rewind();
  while(dir.next_entry()){
      if(dir.isFile()){
          if(!first) server.sendContent(",");
          server.sendContent("{\"type\":\"file\",\"name\":\"" + dir.fileName() + "\"}");
          first = false;
      }
  }

  server.sendContent("]");
}

int main() {
    // Setup data
    mockDir.addDir("dir1");
    mockDir.addDir("dir2");
    mockDir.addFile("file1.txt");
    mockDir.addFile("file2.log");

    printDirectory_Streaming();

    std::cout << "Generated Output:\n" << server.content << std::endl;

    // Verify JSON (simple check)
    std::string expected = "[{\"type\":\"dir\",\"name\":\"esp_spiffs\"},{\"type\":\"dir\",\"name\":\"dir1\"},{\"type\":\"dir\",\"name\":\"dir2\"},{\"type\":\"file\",\"name\":\"file1.txt\"},{\"type\":\"file\",\"name\":\"file2.log\"}]";

    if (server.content == expected) {
        std::cout << "SUCCESS: Output matches expected JSON." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: Output mismatch." << std::endl;
        std::cout << "Expected: " << expected << std::endl;
        return 1;
    }
}
