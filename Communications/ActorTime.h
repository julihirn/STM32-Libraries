#ifndef ACTOR_TIME_H_
    #define ACTOR_TIME_H_
    #include "Communications.h"
    #if ACTOR_ALLOW_TIME == 1
        struct TimeStruct{
            uint8_t Year;
            uint8_t Month;
            uint8_t Day;
            uint8_t Hours;
            uint8_t Minutes;
            uint8_t Seconds;
        };
        enum class DateComponents{
            Seconds = 0x01,
            Minutes = 0x02,
            Hours = 0x04,
            Days = 0x08,
            Months = 0x10,
            Years = 0x20
        };
        constexpr enum DateComponents operator |( const enum DateComponents selfValue, const enum DateComponents inValue ){
            return (enum DateComponents)(uint8_t(selfValue) | uint8_t(inValue));
        }
        constexpr enum DateComponents operator &( const enum DateComponents selfValue, const enum DateComponents inValue ){
            return (enum DateComponents)(uint8_t(selfValue) & uint8_t(inValue));
        }
        namespace ActorTime{
            bool IsTimeInRange(TimeStruct * Now, TimeStruct * RangeStart, TimeStruct * RangeEnd);
            bool IsDateLater(TimeStruct * StartDate, TimeStruct * CompareWith);
            bool IsDateEarlier(TimeStruct * StartDate, TimeStruct * CompareWith);
            bool IsDateEqual(TimeStruct * Input, TimeStruct * CompareWith);
            bool IsDateInRange(TimeStruct * Now, TimeStruct * RangeStart, TimeStruct * RangeEnd);
            TimeStruct StripDate(TimeStruct * Input, DateComponents ComponentsToRemove);
            bool IsYearLeapYear(uint8_t Year);
            bool IsYearLeapYear(uint16_t Year);
            void CreateDate(TimeStruct * Input, uint16_t Year, uint8_t Month, uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Second);
            bool IsDateInRangeIgnoringYear(TimeStruct * Now, TimeStruct * RangeStart, TimeStruct * RangeEnd);
        };
    #endif
#endif

