const fs = require('fs');
const path = require('path');
const vm = require('vm');

describe('SD/graphoffline.js', () => {
    let context;
    let code;

    beforeAll(() => {
        const filePath = path.resolve(__dirname, '../SD/graphoffline.js');
        code = fs.readFileSync(filePath, 'utf8');
    });

    beforeEach(() => {
        // Create a fresh context for each test to avoid side effects
        const mockJQueryObj = {
            empty: jest.fn().mockReturnThis(),
            append: jest.fn().mockReturnThis(),
            prop: jest.fn().mockReturnThis(),
            change: jest.fn().mockReturnThis(),
            html: jest.fn().mockReturnThis(),
            css: jest.fn().mockReturnThis(),
            on: jest.fn().mockReturnThis(),
            click: jest.fn().mockReturnThis(),
            val: jest.fn().mockReturnThis(),
            show: jest.fn().mockReturnThis(),
            hide: jest.fn().mockReturnThis(),
            datetimepicker: jest.fn().mockReturnThis(),
            data: jest.fn().mockReturnValue({
                minDate: jest.fn(),
                maxDate: jest.fn(),
                date: jest.fn(),
                viewDate: jest.fn().mockReturnValue({
                    format: jest.fn(),
                    diff: jest.fn().mockReturnValue(0)
                })
            }),
            removeClass: jest.fn().mockReturnThis(),
            addClass: jest.fn().mockReturnThis(),
            children: jest.fn().mockReturnThis(),
            first: jest.fn().mockReturnThis(),
            off: jest.fn().mockReturnThis(),
            width: jest.fn().mockReturnValue(100),
            height: jest.fn().mockReturnValue(100),
            bind: jest.fn().mockReturnThis(),
            remove: jest.fn().mockReturnThis(),
            appendTo: jest.fn().mockReturnThis(),
            fadeIn: jest.fn().mockReturnThis(),
            select: jest.fn().mockReturnThis(),
            next: jest.fn().mockReturnThis(),
            attr: jest.fn().mockReturnValue('1'), // dummy return
            resize: jest.fn().mockReturnThis(),
            keyup: jest.fn().mockReturnThis(),
        };

        const mockJQuery = jest.fn(() => mockJQueryObj);
        mockJQuery.ajax = jest.fn();
        mockJQuery.isNumeric = jest.fn().mockReturnValue(true);
        mockJQuery.plot = jest.fn();

        context = {
            window: {
                location: { search: '', host: 'localhost' },
                resize: jest.fn(),
                width: jest.fn().mockReturnValue(1024),
                height: jest.fn().mockReturnValue(768),
            },
            document: {
                execCommand: jest.fn(),
                body: {
                    appendChild: jest.fn(),
                    removeChild: jest.fn(),
                },
                createElement: jest.fn().mockReturnValue({ style: {}, setAttribute: jest.fn(), click: jest.fn() }),
                getElementById: jest.fn().mockReturnValue({ select: jest.fn() }),
                title: ""
            },
            $: mockJQuery,
            jQuery: mockJQuery,
            URLSearchParams: URLSearchParams,
            console: console,
            moment: jest.fn().mockReturnValue({ format: jest.fn().mockReturnValue('mocked date') }),
            alert: jest.fn(),
            setTimeout: jest.fn(),
            clearTimeout: jest.fn(),
            location: { host: 'localhost', search: '' }
        };

        // Add moment methods
        context.moment.unix = jest.fn().mockReturnValue({ format: jest.fn() });

        vm.createContext(context);
        vm.runInContext(code, context);
    });

    describe('unitFormat', () => {
        test('formats Volts correctly', () => {
            expect(context.unitFormat(230.123, 'Volts')).toBe('230.1 V');
            expect(context.unitFormat(230.678, 'Volts')).toBe('230.7 V');
            expect(context.unitFormat(0, 'Volts')).toBe('0.0 V');
        });

        test('formats PF (Power Factor) correctly', () => {
            // > 0.999
            expect(context.unitFormat(1.000, 'PF')).toBe('1.00 PF');
            expect(context.unitFormat(1.05, 'PF')).toBe('1.05 PF');

            // <= 0.999
            expect(context.unitFormat(0.999, 'PF')).toBe('.999 PF');
            expect(context.unitFormat(0.950, 'PF')).toBe('.950 PF');
            expect(context.unitFormat(0.123, 'PF')).toBe('.123 PF');
        });

        test('formats Hz correctly', () => {
            expect(context.unitFormat(50.123, 'Hz')).toBe('50.12 Hz');
            expect(context.unitFormat(60, 'Hz')).toBe('60.00 Hz');
        });

        test('formats Watts with scaling', () => {
            const unit = "Watts";
            // Check formats table logic
            // { max: 1, div: 0.001, prefix: "m", dp: 0 }
            expect(context.unitFormat(0.5, unit)).toBe('500 mW');

            // { max: 10, div: 1, prefix: "", dp: 2 }
            expect(context.unitFormat(5.123, unit)).toBe('5.12 W');

            // { max: 100, div: 1, prefix: "", dp: 1 }
            expect(context.unitFormat(50.123, unit)).toBe('50.1 W');

            // { max: 1000, div: 1, prefix: "", dp: 0 }
            expect(context.unitFormat(500.123, unit)).toBe('500 W');

            // { max: 10000, div: 1000, prefix: "k", dp: 2 }
            expect(context.unitFormat(2500, unit)).toBe('2.50 kW');

            // { max: 100000, div: 1000, prefix: "k", dp: 1 }
            expect(context.unitFormat(25000, unit)).toBe('25.0 kW');

            // { max: 1000000, div: 1000, prefix: "k", dp: 0 }
            expect(context.unitFormat(500000, unit)).toBe('500 kW');

            // { max: 10000000, div: 1000000, prefix: "M", dp: 2 }
            expect(context.unitFormat(2500000, unit)).toBe('2.50 MW');
        });

        test('formats Wh with scaling', () => {
            const unit = "Wh";
            // Similar logic but unit label is Wh
            expect(context.unitFormat(2500, unit)).toBe('2.50 kWh');
        });

        test('formats Amps with scaling', () => {
            const unit = "Amps";
            // Amps label is A
            expect(context.unitFormat(5.123, unit)).toBe('5.12 A');
            expect(context.unitFormat(100, unit)).toBe('100 A'); // max 1000, dp 0
        });

        test('formats 0 for scaled units', () => {
            expect(context.unitFormat(0, 'Watts')).toBe('0 W');
            expect(context.unitFormat(0, 'Amps')).toBe('0 A');
        });

        test('formats unknown unit', () => {
             expect(context.unitFormat(123, 'Unknown')).toBe('123 Unknown');
        });

        test('formats negative values correctly', () => {
             // Math.abs(val) is used for scaling check
             expect(context.unitFormat(-500, 'Watts')).toBe('-500 W');
             expect(context.unitFormat(-2500, 'Watts')).toBe('-2.50 kW');
        });
    });
});
