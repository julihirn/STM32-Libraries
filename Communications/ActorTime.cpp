#include "ActorTime.h"

bool ActorTime::IsTimeInRange(TimeStruct *Now, TimeStruct * RangeStart, TimeStruct * RangeEnd){
    int SH = RangeStart->Hours;
    int SM = RangeStart->Minutes;
    int EH = RangeEnd->Hours;
    int EM = RangeEnd->Minutes;
    bool ret = false;
    char CurrentHour = Now->Hours;
    char CurrentMinute = Now->Minutes;
    if (SH > EH){
        if (CurrentHour==SH){
            if(CurrentMinute>=SM){ ret = true;}
        }
        else if (CurrentHour > SH){ ret = true;}
        if (CurrentHour==EH){
            if(CurrentMinute < EM){ ret = true;}
        }
        else if (CurrentHour < EH){ ret = true;}
    }
    else if (SH == EH){
        if (CurrentHour == SH){
            if ((CurrentMinute >= SM) && (CurrentMinute < EM)){ ret = true;} 
        }
    }
    else{
        if (CurrentHour==EH){
            if(CurrentMinute>=EM){ ret = true;}
        }
        else if (CurrentHour > EH){ ret = true;}
        if (CurrentHour==SH){
            if(CurrentMinute < SM){ ret = true;}
        }
        else if (CurrentHour < SH){ ret = true;}
    }
    return ret;
}
bool ActorTime::IsDateLater(TimeStruct* StartDate, TimeStruct* CompareWith){
    if (CompareWith->Year > StartDate->Year) {
        return true;
    }
    else if (CompareWith->Year == StartDate->Year) {
        if (CompareWith->Month > StartDate->Month) {
            return true;
        }
        else if (CompareWith->Month == StartDate->Month) {
            if (CompareWith->Day > StartDate->Day) {
                return true;
            }
            else if (CompareWith->Day == StartDate->Day) {
                if (CompareWith->Hours > StartDate->Hours) {
                    return true;
                }
                else if (CompareWith->Hours == StartDate->Hours) {
                    if (CompareWith->Minutes > StartDate->Minutes) {
                        return true;
                    }
                    else if (CompareWith->Minutes == StartDate->Minutes) {
                        if (CompareWith->Seconds > StartDate->Seconds) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
bool ActorTime::IsDateEarlier(TimeStruct* StartDate, TimeStruct* CompareWith){
    if (CompareWith->Year < StartDate->Year) {
        return true;
    }
    else if (CompareWith->Year == StartDate->Year) {
        if (CompareWith->Month < StartDate->Month) {
            return true;
        }
        else if (CompareWith->Month == StartDate->Month) {
            if (CompareWith->Day < StartDate->Day) {
                return true;
            }
            else if (CompareWith->Day == StartDate->Day) {
                if (CompareWith->Hours < StartDate->Hours) {
                    return true;
                }
                else if (CompareWith->Hours == StartDate->Hours) {
                    if (CompareWith->Minutes < StartDate->Minutes) {
                        return true;
                    }
                    else if (CompareWith->Minutes == StartDate->Minutes) {
                        if (CompareWith->Seconds < StartDate->Seconds) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
bool ActorTime::IsDateEqual(TimeStruct* Input, TimeStruct* CompareWith) {
    return (Input->Year == CompareWith->Year) &&
           (Input->Month == CompareWith->Month) &&
           (Input->Day == CompareWith->Day) &&
           (Input->Hours == CompareWith->Hours) &&
           (Input->Minutes == CompareWith->Minutes) &&
           (Input->Seconds == CompareWith->Seconds);
}
bool ActorTime::IsDateInRange(TimeStruct * Now, TimeStruct * RangeStart, TimeStruct * RangeEnd){
    bool IsEndDateLater = IsDateLater(Now, RangeEnd);
    bool IsStartDateEarlier = IsDateEarlier(Now, RangeStart);
    return IsEndDateLater == IsStartDateEarlier ? true : false;
}

TimeStruct ActorTime::StripDate(TimeStruct * Input, DateComponents ComponentsToRemove){
    TimeStruct Temp;
    Temp.Year       = (ComponentsToRemove & DateComponents::Years)   == DateComponents::Years   ? 0x00 : Input->Year;
    Temp.Month      = (ComponentsToRemove & DateComponents::Months)  == DateComponents::Months  ? 0x00 : Input->Month;
    Temp.Day        = (ComponentsToRemove & DateComponents::Days)    == DateComponents::Days    ? 0x00 : Input->Day;
    Temp.Hours      = (ComponentsToRemove & DateComponents::Hours)   == DateComponents::Hours   ? 0x00 : Input->Hours;
    Temp.Minutes    = (ComponentsToRemove & DateComponents::Minutes) == DateComponents::Minutes ? 0x00 : Input->Minutes;
    Temp.Seconds    = (ComponentsToRemove & DateComponents::Seconds) == DateComponents::Seconds ? 0x00 : Input->Seconds;
    return Temp;
}

bool ActorTime::IsYearLeapYear(uint8_t Year){
    uint16_t TempYear = 2000 + Year;
    return IsYearLeapYear(TempYear);
}
bool ActorTime::IsYearLeapYear(uint16_t Year){
    if (Year % 4 == 0){
        if (Year % 100 == 0){
            if (Year % 400 == 0){
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

void ActorTime::CreateDate(TimeStruct * Input, uint16_t Year, uint8_t Month, uint8_t Day, uint8_t Hour, uint8_t Minute, uint8_t Second){
    if ((Year >= 2000) && (Year < 2255)){
        Input->Year = (uint8_t)(Year - 2000);
    }
    else{Input->Year = 0;}
    if ((Month >= 1) && (Month <= 12)){Input->Month = Month;}
    else{
        if (Month ==0){Input->Month = 1;}
        else{Input->Month = 12;}
    }
    if (Day == 0){Input->Day = 1;}
    if (Month == 2){
        if (IsYearLeapYear(Year) == true){
            if (Day >= 30){Input->Day = 29;}
            else{Input->Day = Day;}
        }
        else{
            if (Day >= 29){Input->Day = 28;}
            else{Input->Day = Day;}
        }
    }
    else if((Month == 4) || (Month==6) || (Month==9) || (Month==11)){
        if (Day > 31){Input->Day = 30;}
        else{Input->Day = Day;}
    }
    else{
        if (Day > 32){Input->Day = 31;}
        else{Input->Day = Day;}
    }
    if ((Hour >= 0)&&(Hour < 24)){Input->Hours = Hour;}
    else{Input->Hours = 23;}
    if ((Minute >= 0)&&(Minute < 60)){Input->Minutes = Minute;}
    else{Input->Minutes = 59;}
    if ((Second >= 0)&&(Second < 60)){Input->Seconds = Second;}
    else{Input->Seconds = 59;}
}
bool ActorTime::IsDateInRangeIgnoringYear(TimeStruct * Now, TimeStruct * RangeStart, TimeStruct * RangeEnd){
    TimeStruct TempNow = StripDate(Now, DateComponents::Years);
    TimeStruct TempStart = StripDate(RangeStart, DateComponents::Years);
    TimeStruct TempEnd = StripDate(RangeEnd, DateComponents::Years);
    if (RangeStart->Month > RangeEnd->Month){
        TempEnd.Year++;
        if (RangeStart->Month <= Now->Month){}
        else{TempNow.Year++;}
    }
    bool IsEndDateLater = IsDateLater(&TempNow, &TempEnd);
    bool IsStartDateEarlier = IsDateEarlier(&TempNow, &TempStart);
    return IsEndDateLater == IsStartDateEarlier ? true : IsDateEqual(&TempNow, &TempStart);
}

