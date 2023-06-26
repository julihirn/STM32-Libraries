#include "Actor.h"
#if USE_ACTORS == 1
    Actor::Actor(){
        //this->Command = "";
        this->Direction = ActorDirection::Output;
        this->Type = ActorDataType::Float;
    }
    Actor::Actor(String Name, ActorDirection Direction, ActorDataType Type){
        this->Command = Name;
        this->Direction = Direction;
        this->Type = Type;
        AllocateMemory(this, Type);
    }
    Actor::Actor(String Name, uint8_t ID, ActorDirection Direction, ActorDataType Type){
        this->Command = Name;
        this->UniqueID = ID;
        this->Direction = Direction;
        this->Type = Type;
        AllocateMemory(this, Type);
    }
    Actor::Actor(String Name, ActorDirection Direction, ActorDataType Type, void (*OnChange)(Actor *)){
        OnChangeEvent = OnChange;
        this->Flags |= FLAG_CANTRIGGERFUNCTION;
        this->Command = Name;
        this->Direction = Direction;
        this->Type = Type;
        AllocateMemory(this, Type);
    }
    Actor::Actor(String Name, uint8_t ID, ActorDirection Direction, ActorDataType Type, void (*OnChange)(Actor *)){
        OnChangeEvent = OnChange;
        this->Flags |= FLAG_CANTRIGGERFUNCTION;
        this->Command = Name;
        this->Direction = Direction;
        this->Type = Type;
        this->UniqueID = ID;
        AllocateMemory(this, Type);
    }
    Actor::~Actor(){
        free(Data);
    }

    ActorState Actor::SetData(String & Data){
        ActorState ret = ACTOR_OK;
        if(Data == NULL){ return ACTOR_TYPE_ERROR;}
        if (this->Type == ActorDataType::Bool){
            String Temp = Data;
            Temp.toLowerCase();
            bool Val = false;
            if (Temp == "t"){ Val = true;}
            else if (Temp == "1"){ Val = true;}
            else if (Temp == Defines::CONST_TRUE){ Val = true;}
            SetData(Val);
        }
        #if ACTOR_ALLOW_INT8
            else if (this->Type == ActorDataType::Int8){
                int8_t Temp = (int8_t)Data.toInt();
                SetBaseData(this, Temp);
            }
        #endif
        #if ACTOR_ALLOW_INT16
            else if (this->Type == ActorDataType::Int16){
                int16_t Temp = (int16_t)Data.toInt();
                SetBaseData(this, Temp);
            }
        #endif
        #if ACTOR_ALLOW_INT32
            else if (this->Type == ActorDataType::Int32){
                int32_t Temp = (int32_t)Data.toInt();
                SetBaseData(this, Temp);
            }
        #endif
        #if ACTOR_ALLOW_FLOATS
            else if (this->Type == ActorDataType::Float){
                float Temp = Data.toFloat();
                SetBaseData(this, Temp);
            }
        #endif
        #if ACTOR_ALLOW_TIME == 1
            else if (this->Type == ActorDataType::Time){
                String Temp = Data;
                TimeStruct TempTime;
                //1234567890123
                //0123456789012
                //YYYMMDDHHMMSS;
                if (Temp.length() == 13){
                    TempTime.Year = (uint8_t)Temp.substring(0,3).toInt();
                    TempTime.Month = (uint8_t)Temp.substring(3,5).toInt();
                    TempTime.Day = (uint8_t)Temp.substring(5,7).toInt();
                    TempTime.Hours = (uint8_t)Temp.substring(7,9).toInt();
                    TempTime.Minutes = (uint8_t)Temp.substring(9,11).toInt();
                    TempTime.Seconds = (uint8_t)Temp.substring(11,13).toInt();
                    #if ACTOR_DEBUG_MESSAGES == 1
                        printf("Actor: Data committed with value '%u/%u/%u %u:%u:%u'\n", TempTime.Day,TempTime.Month, TempTime.Year, TempTime.Day, TempTime.Hours,TempTime.Minutes,TempTime.Seconds);
			        #endif 
                    return SetData(&TempTime);
                }
                else{
                    #if ACTOR_DEBUG_MESSAGES == 1
				        printf("Actor: Date string formatting mismatched. Expected: 'YYYMMDDHHMMSS'\n");
			        #endif 
                    return ACTOR_MISMATCH;
                }
            }
        #endif
        else if(this->Type == ActorDataType::Void){
            this->Flags |= FLAG_ISMODIFIED;
            #if ACTOR_DEBUG_MESSAGES == 1
                printf("Actor: Invoked'\n");
            #endif
        }
        else if(this->Type == ActorDataType::PassThrough){
            this->Flags |= FLAG_ISMODIFIED;
            *((String *)this->Data) = Data;
            #if ACTOR_DEBUG_MESSAGES == 1
                printf("Actor: Invoked and Data is ready!'\n");
            #endif
        }
        else{ ret = ACTOR_TYPE_ERROR; }
        #if ACTOR_DEBUG_MESSAGES == 1
            if(ret == ACTOR_TYPE_ERROR){
                printf("Actor: Data commit failed! Type or data invalid");
            }
		#endif
        return ret;
    }

    
    
    
    ActorState Actor::SetData(bool Data){
        ActorState ret = ACTOR_OK;
        if (this->Type == ActorDataType::Bool){
            bool Temp = *((bool *)this->Data);
            #if ACTOR_MARK_DIFFERENCE_ONLY == 1
                if (Data != Temp){this->Flags |= FLAG_ISMODIFIED;}
            #else
                this->Flags |= FLAG_ISMODIFIED;
            #endif
            *((bool *)this->Data) = Data;
            #if ACTOR_DEBUG_MESSAGES == 1
                if(this->DigitalData == true){printf("Actor: Data committed with value 'true''\n");}
                else{printf("Actor: Data committed with value 'false''\n");}
			#endif 
        }
        else if(this->Type == ActorDataType::Void){
            this->Flags |= FLAG_ISMODIFIED;

            #if ACTOR_DEBUG_MESSAGES == 1
                printf("Actor: Invoked'\n");
            #endif
        }
        else{ ret = ACTOR_TYPE_ERROR; }
        #if ACTOR_DEBUG_MESSAGES == 1
            if(ret == ACTOR_TYPE_ERROR){
                printf("Actor: Data commit failed! Type != Digital");
            }
		#endif
        return ret;
    }
    ActorState Actor::SetData(bool Data, bool Trigger){ 
        ActorState ret = SetData(Data);
        TriggerOnChangeEvent(Trigger);
        return ret;
    }

    ActorState Actor::SetData(String & Data, bool Trigger){
        ActorState ret = SetData(Data);
        TriggerOnChangeEvent(Trigger);
        return ret;
    }
    
    #if ACTOR_ALLOW_TIME == 1
        ActorState Actor::SetData(TimeStruct * InputTime, bool Trigger){ 
            ActorState ret = SetData(InputTime);
            TriggerOnChangeEvent(Trigger);
            return ret;
        }
        ActorState Actor::SetData(TimeStruct * InputTime){
            TimeStruct Temp;
            if((InputTime->Month>=1)&&(InputTime->Month<13)){Temp.Month = InputTime->Month;}
            else{return ACTOR_OUT_OF_RANGE;}
            //Days
            if ((InputTime->Day<1)&&(InputTime->Day>31)){return ACTOR_OUT_OF_RANGE;}
            if (Temp.Month == 2){
                if (ActorTime::IsYearLeapYear(InputTime->Year) == true){
                    if (InputTime->Day > 30){return ACTOR_OUT_OF_RANGE;}
                    else{Temp.Day = InputTime->Day;}
                }
                else{
                    if (InputTime->Day > 29){return ACTOR_OUT_OF_RANGE;}
                    else{Temp.Day = InputTime->Day;}
                }
            }
            else if((Temp.Month == 4) || (Temp.Month==6) || (Temp.Month==9) || (Temp.Month==11)){
                if (InputTime->Day > 31){return ACTOR_OUT_OF_RANGE;}
                else{Temp.Day = InputTime->Day;}
            }
            else{
                if (InputTime->Day > 32){return ACTOR_OUT_OF_RANGE;}
                else{Temp.Day = InputTime->Day;}
            }
            //Hours
            if(InputTime->Hours<24){Temp.Hours = InputTime->Hours;}
            else{return ACTOR_OUT_OF_RANGE;}
            //Minutes
            if(InputTime->Minutes<60){Temp.Minutes = InputTime->Minutes;}
            else{return ACTOR_OUT_OF_RANGE;}
            //Seconds
            if(InputTime->Seconds<60){Temp.Seconds = InputTime->Seconds;}
            else{return ACTOR_OUT_OF_RANGE;}
            ((TimeStruct *)this->Data)->Year=InputTime->Year;
            ((TimeStruct *)this->Data)->Month=Temp.Month;
            ((TimeStruct *)this->Data)->Day=Temp.Day;
            ((TimeStruct *)this->Data)->Hours=Temp.Hours;
            ((TimeStruct *)this->Data)->Minutes=Temp.Minutes;
            ((TimeStruct *)this->Data)->Seconds=Temp.Seconds;
            this->Flags |= FLAG_ISMODIFIED;
            return ACTOR_OK;
        }
    #endif
    String * Actor::GetName(){ return &Command; }
    
    
    //Getters
    ActorState Actor::GetData(bool * Data){
        ActorState ret = ACTOR_OK;
        if (this->Type == ActorDataType::Bool){
            * Data = *((bool *)this->Data);
            if((Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
        }
        else{ ret = ACTOR_TYPE_ERROR; }
        return ret;
    }
    #if ACTOR_ALLOW_INT8 == 1
        ActorState Actor::GetData(char * Data){
            int8_t Temp = 0;
            ActorState State = GetBaseInt8(this, &Temp);
            *Data = (char)Temp;
            return State;
        }
        ActorState Actor::GetData(uint8_t * Data){
            int8_t Temp = 0;
            ActorState State = GetBaseInt8(this, &Temp);
            *Data = (uint8_t)Temp;
            return State;
        }
        ActorState Actor::GetData(int8_t * Data){
            return GetBaseInt8(this, Data);
        }
    #endif
    #if ACTOR_ALLOW_INT16 == 1
        ActorState Actor::GetData(int * Data){
            int16_t Temp = 0;
            ActorState State = GetBaseInt16(this, &Temp);
            *Data = (int)Temp;
            return State;
        }
        ActorState Actor::GetData(uint16_t * Data){
            int16_t Temp = 0;
            ActorState State = GetBaseInt16(this, &Temp);
            *Data = (uint16_t)Temp;
            return State;
        }
        ActorState Actor::GetData(int16_t * Data){
            return GetBaseInt16(this, Data);
        }
    #endif
    #if ACTOR_ALLOW_INT32 == 1
        ActorState Actor::GetData(long * Data){
            int32_t Temp = 0;
            ActorState State = GetBaseInt32(this, &Temp);
            *Data = (long)Temp;
            return State;
        }
        ActorState Actor::GetData(uint32_t * Data){
            int32_t Temp = 0;
            ActorState State = GetBaseInt32(this, &Temp);
            *Data = (uint32_t)Temp;
            return State;
        }
        ActorState Actor::GetData(int32_t * Data){
            return GetBaseInt32(this, Data);
        }
    #endif
    #if ACTOR_ALLOW_FLOATS == 1
        ActorState Actor::GetData(float * Data){
            ActorState ret = ACTOR_OK;
            if (this->Type == ActorDataType::Float){
                * Data = *((float *)this->Data);
                if((Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            return ret;
        }
    #endif
    #if ACTOR_ALLOW_TIME == 1
        ActorState Actor::GetData(TimeStruct * Data){
            ActorState ret = ACTOR_OK;
            if (this->Type == ActorDataType::Time){
                TimeStruct * Temp = ((TimeStruct *)this->Data);
            	Data->Year  = Temp->Year;
            	Data->Month = Temp->Month;
            	Data->Day   = Temp->Day;
            	Data->Hours = Temp->Hours;
            	Data->Minutes = Temp->Minutes;
            	Data->Seconds = Temp->Seconds;
                if((Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            return ret;
        }
    #endif

    ActorState Actor::GetData(String * Data){
        ActorState ret = ACTOR_OK;
        if (this->Type == ActorDataType::PassThrough){
            * Data = *((String *)this->Data);
            if((Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
        }
        else{ ret = ACTOR_TYPE_ERROR; }
        return ret;
    }
    
    //Setters
    #if ACTOR_ALLOW_INT8 == 1
        ActorState Actor::SetData(int8_t Data){
            #if ACTOR_ALLOW_INT8 == 1
                if (this->Type == ActorDataType::Int8){
                    return SetBaseData(this, Data);
                }
            #endif
            #if ACTOR_ALLOW_INT16 == 1
                if (this->Type == ActorDataType::Int16){
                    return SetBaseData(this, (int16_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT32 == 1
                if (this->Type == ActorDataType::Int32){
                    return SetBaseData(this, (int32_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_FLOATS == 1
                if (this->Type == ActorDataType::Float){
                    return SetBaseData(this, (float)Data);
                }
            #endif
            return ActorState::ACTOR_TYPE_ERROR;
        }
        ActorState Actor::SetData(uint8_t Data){
            return SetData((int8_t)Data);
        }
        ActorState Actor::SetData(char Data){
            return SetData((int8_t)Data);
        }
        ActorState Actor::SetData(int8_t Data, bool Trigger){
            ActorState ret = SetData(Data);
            TriggerOnChangeEvent(Trigger);
            return ret;
        }
        ActorState Actor::SetData(uint8_t Data, bool Trigger){
            return SetData((int8_t)Data, Trigger);
        }
        ActorState Actor::SetData(char Data, bool Trigger){
            return SetData((int8_t)Data, Trigger);
        }
    #endif
    #if ACTOR_ALLOW_INT16 == 1
        ActorState Actor::SetData(int16_t Data){
            #if ACTOR_ALLOW_INT8 == 1
                if (this->Type == ActorDataType::Int8){
                    return SetBaseData(this, (int8_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT16 == 1
                if (this->Type == ActorDataType::Int16){
                    return SetBaseData(this, Data);
                }
            #endif
            #if ACTOR_ALLOW_INT32 == 1
                if (this->Type == ActorDataType::Int32){
                    return SetBaseData(this, (int32_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_FLOATS == 1
                if (this->Type == ActorDataType::Float){
                    return SetBaseData(this, (float)Data);
                }
            #endif
            return ActorState::ACTOR_TYPE_ERROR;
        }
        ActorState Actor::SetData(uint16_t Data){
            return SetData((int16_t)Data);
        }
        ActorState Actor::SetData(int Data){
            return SetData((int16_t)Data);
        }
        ActorState Actor::SetData(int16_t Data, bool Trigger){
            ActorState ret = SetData(Data);
            TriggerOnChangeEvent(Trigger);
            return ret;
        }
        ActorState Actor::SetData(uint16_t Data, bool Trigger){
            return SetData((int16_t)Data, Trigger);
        }
        ActorState Actor::SetData(int Data, bool Trigger){
            return SetData((int16_t)Data, Trigger);
        }
    #endif
    #if ACTOR_ALLOW_INT32 == 1
        ActorState Actor::SetData(int32_t Data){
            #if ACTOR_ALLOW_INT8 == 1
                if (this->Type == ActorDataType::Int8){
                    return SetBaseData(this, (int8_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT16 == 1
                if (this->Type == ActorDataType::Int16){
                    return SetBaseData(this, (int16_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT32 == 1
                if (this->Type == ActorDataType::Int32){
                    return SetBaseData(this, Data);
                }
            #endif
            #if ACTOR_ALLOW_FLOATS == 1
                if (this->Type == ActorDataType::Float){
                    return SetBaseData(this, (float)Data);
                }
            #endif
            return ActorState::ACTOR_TYPE_ERROR;
        }
        ActorState Actor::SetData(uint32_t Data){
            return SetData((int32_t)Data);
        }
        ActorState Actor::SetData(long Data){
            return SetData((int32_t)Data);
        }
        ActorState Actor::SetData(int32_t Data, bool Trigger){
            ActorState ret = SetData(Data);
            TriggerOnChangeEvent(Trigger);
            return ret;
        }
        ActorState Actor::SetData(uint32_t Data, bool Trigger){
            return SetData((int32_t)Data, Trigger);
        }
        ActorState Actor::SetData(long Data, bool Trigger){
            return SetData((int32_t)Data, Trigger);
        }
    #endif
    #if ACTOR_ALLOW_FLOATS == 1
        ActorState Actor::SetData(float Data){
            #if ACTOR_ALLOW_INT8 == 1
                if (this->Type == ActorDataType::Int8){
                    return SetBaseData(this, (int8_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT16 == 1
                if (this->Type == ActorDataType::Int16){
                    return SetBaseData(this, (int16_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_INT32 == 1
                if (this->Type == ActorDataType::Int32){
                    return SetBaseData(this, (int32_t)Data);
                }
            #endif
            #if ACTOR_ALLOW_FLOATS == 1
                if (this->Type == ActorDataType::Float){
                    return SetBaseData(this, Data);
                }
            #endif
            return ActorState::ACTOR_TYPE_ERROR;
        }
        ActorState Actor::SetData(float Data, bool Trigger){
            ActorState ret = SetData(Data);
            TriggerOnChangeEvent(Trigger);
            return ret;
        }
    #endif


    bool Actor::GetState(){
        if((Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){return true;}
    	return false;
    }
    void Actor::ResetState(){
    	//IsModified = false;
        Flags &=~FLAG_ISMODIFIED;
        #if ACTOR_DEBUG_MESSAGES == 1
            printf("Actor: Flag 'ISMODIFIED' was reset\n");
		#endif 
    }
    ActorDirection Actor::GetDirection(){
    	return Direction;
    }
    ActorDataType Actor::GetType(){
        return Type;
    }
    
    
    #if USE_ADDRESSING == 1
    ActorState Actor::BuildCommandString(String * Command, unsigned long *id){
    #else
    ActorState Actor::BuildCommandString(String * Command){
    #endif
    	ActorState ret = ACTOR_OK;
    	String * Output = Command;
		#if USE_ADDRESSING == 1
            if(id == NULL){return ActorState::ACTOR_OUT_OF_RANGE;}
    		*Output = String(*id) + Defines::Comma + this->Command + '=';
		#else
    		*Output = (this->Command) + '=';
		#endif
    	if(this->Type == ActorDataType::Bool){
    		bool BufBool = false;
    		if (GetData(&BufBool) == ACTOR_TYPE_ERROR){
    			ret = ACTOR_BUILD_ERROR;
    		}
            if(BufBool==true){*Output = *Output + Defines::CharTrue;}
            else{*Output = *Output + Defines::CharFalse;}
    	}
        #if ACTOR_ALLOW_INT8 == 1
            else if(this->Type == ActorDataType::Int8){
    	    	int8_t Buf = 0;
    	    	if (GetData(&Buf) == ACTOR_TYPE_ERROR){ ret = ACTOR_BUILD_ERROR;}
    	    	*Output = *Output + String((int)Buf,3);
    	    }
        #endif
        #if ACTOR_ALLOW_INT16 == 1
            else if(this->Type == ActorDataType::Int16){
    	    	int16_t Buf = 0;
    	    	if (GetData(&Buf) == ACTOR_TYPE_ERROR){ ret = ACTOR_BUILD_ERROR;}
    	    	*Output = *Output + String((int)Buf,3);
    	    }
        #endif
        #if ACTOR_ALLOW_INT32 == 1
            else if(this->Type == ActorDataType::Int32){
    	    	int32_t Buf = 0;
    	    	if (GetData(&Buf) == ACTOR_TYPE_ERROR){ ret = ACTOR_BUILD_ERROR;}
    	    	*Output = *Output + String((long)Buf,3);
    	    }
        #endif
        #if ACTOR_ALLOW_FLOATS == 1
            else if(this->Type == ActorDataType::Float){
    	    	float BufDec = 0;
    	    	if (GetData(&BufDec) == ACTOR_TYPE_ERROR){ ret = ACTOR_BUILD_ERROR;}
    	    	*Output = *Output + String(BufDec,3);
    	    }
        #endif
        #if ACTOR_ALLOW_TIME == 1
            else if(this->Type == ActorDataType::Time){
                TimeStruct Ts; 
                if (GetData(&Ts) == ACTOR_TYPE_ERROR){ ret = ACTOR_BUILD_ERROR;}
                FormatInteger(Output, Ts.Year,3);
                FormatInteger(Output, Ts.Month,2);
                FormatInteger(Output, Ts.Day,2);
                FormatInteger(Output, Ts.Hours,2);
                FormatInteger(Output, Ts.Minutes,2);
                FormatInteger(Output, Ts.Seconds,2);
                //*Output = *Output + Defines::Term;
            }
        #endif
    	else{ ret = ACTOR_NOT_HANDLED; }
    	return ret;
    }


    void Actor::TriggerOnChangeEvent(bool Trigger){
        if ((Flags&FLAG_CANTRIGGERFUNCTION)!=FLAG_CANTRIGGERFUNCTION){return;}
        if (Trigger == false){return;}
        if (*OnChangeEvent == NULL){return;}
        OnChangeEvent(this);
    }

    
    void Actor::Invalidate(void){
        this->Flags |=FLAG_ISMODIFIED;
    }

    //Base Data Retrieval
    #if ACTOR_ALLOW_INT8 == 1
        ActorState Actor::GetBaseInt8(Actor * Input, int8_t * Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int8){
                * Data = *((int8_t *)Input->Data);
                if((Input->Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Input->Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
            }
            else{ ret = ACTOR_TYPE_ERROR; }
        return ret;
    }
    #endif
    #if ACTOR_ALLOW_INT16 == 1
        ActorState Actor::GetBaseInt16(Actor * Input, int16_t * Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int16){
                * Data = *((int16_t *)Input->Data);
                if((Input->Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Input->Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
            }
            else{ ret = ACTOR_TYPE_ERROR; }
        return ret;
    }
    #endif
    #if ACTOR_ALLOW_INT32 == 1
        ActorState Actor::GetBaseInt32(Actor * Input, int32_t * Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int32){
                * Data = *((int32_t *)Input->Data);
                if((Input->Flags&FLAG_ISMODIFIED)==FLAG_ISMODIFIED){ ret = (Input->Direction == ActorDirection::Output) ? ACTOR_OUT_CHANGED : ACTOR_IN_CHANGED; }
            }
            else{ ret = ACTOR_TYPE_ERROR; }
        return ret;
    }
    #endif

    //Base Data Setters
    #if ACTOR_ALLOW_INT8 == 1
        ActorState Actor::SetBaseData(Actor * Input, int8_t Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int8){
                int8_t Temp = *((int8_t *)Input->Data);
                #if ACTOR_MARK_DIFFERENCE_ONLY == 1
                    if (Data != Temp){Input->Flags |= FLAG_ISMODIFIED;}
                #else
                    Input->Flags |= FLAG_ISMODIFIED;
                #endif
                *((int8_t *)Input->Data) = Data;
                #if ACTOR_DEBUG_MESSAGES == 1
		    	    printf("Actor: Data committed with value %d\n", (int)Data);
		    	#endif  
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            #if ACTOR_DEBUG_MESSAGES == 1
                if(ret == ACTOR_TYPE_ERROR){
                    printf("Actor: Data commit failed! Type != Int 8");
                }
		    #endif
            return ret;
        }
    #endif
    #if ACTOR_ALLOW_INT16 == 1
        ActorState Actor::SetBaseData(Actor * Input, int16_t Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int16){
                int16_t Temp = *((int16_t *)Input->Data);
                #if ACTOR_MARK_DIFFERENCE_ONLY == 1
                    if (Data != Temp){Input->Flags |= FLAG_ISMODIFIED;}
                #else
                    Input->Flags |= FLAG_ISMODIFIED;
                #endif
                *((int16_t *)Input->Data) = Data;
                #if ACTOR_DEBUG_MESSAGES == 1
		    	    printf("Actor: Data committed with value %d\n", (int)Data);
		    	#endif  
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            #if ACTOR_DEBUG_MESSAGES == 1
                if(ret == ACTOR_TYPE_ERROR){
                    printf("Actor: Data commit failed! Type != Int 16");
                }
		    #endif
            return ret;
        }
    #endif
    #if ACTOR_ALLOW_INT32 == 1
        ActorState Actor::SetBaseData(Actor * Input, int32_t Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Int16){
                int32_t Temp = *((int32_t *)Input->Data);
                #if ACTOR_MARK_DIFFERENCE_ONLY == 1
                    if (Data != Temp){Input->Flags |= FLAG_ISMODIFIED;}
                #else
                    Input->Flags |= FLAG_ISMODIFIED;
                #endif
                *((int32_t *)Input->Data) = Data;
                #if ACTOR_DEBUG_MESSAGES == 1
		    	    printf("Actor: Data committed with value %d\n", (int)Data);
		    	#endif  
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            #if ACTOR_DEBUG_MESSAGES == 1
                if(ret == ACTOR_TYPE_ERROR){
                    printf("Actor: Data commit failed! Type != Int 32");
                }
		    #endif
            return ret;
        }
    #endif
    #if ACTOR_ALLOW_FLOATS == 1
        ActorState Actor::SetBaseData(Actor * Input, float Data){
            ActorState ret = ACTOR_OK;
            if (Input->Type == ActorDataType::Float){
                float Temp = *((float *)Input->Data);
                #if ACTOR_MARK_DIFFERENCE_ONLY == 1
                    if (Data != Temp){Input->Flags |= FLAG_ISMODIFIED;}
                #else
                    Input->Flags |= FLAG_ISMODIFIED;
                #endif
                *((float *)Input->Data) = Data;
                #if ACTOR_DEBUG_MESSAGES == 1
		    	    printf("Actor: Data committed with value %f\n", Data);
		    	#endif  
            }
            else{ ret = ACTOR_TYPE_ERROR; }
            #if ACTOR_DEBUG_MESSAGES == 1
                if(ret == ACTOR_TYPE_ERROR){
                    printf("Actor: Data commit failed! Type != Float");
                }
		    #endif
            return ret;
        }
    #endif

    void Actor::AllocateMemory(Actor * Input, ActorDataType Type){
        #if ACTOR_ALLOW_TIME == 1
            if (Type == ActorDataType::Time){
                Input->Data = malloc(sizeof(TimeStruct));
            }
        #endif
        if (Type == ActorDataType::Bool){
            Input->Data = malloc(sizeof(bool));
        }
        #if ACTOR_ALLOW_INT8 == 1
            if (Type == ActorDataType::Int8){
                Input->Data = malloc(sizeof(int8_t));
            }
        #endif
        #if ACTOR_ALLOW_INT16 == 1
            if (Type == ActorDataType::Int16){
                Input->Data = malloc(sizeof(int16_t));
            }
        #endif
        #if ACTOR_ALLOW_INT32 == 1
            if (Type == ActorDataType::Int32){
                Input->Data = malloc(sizeof(int32_t));
            }
        #endif
        #if ACTOR_ALLOW_FLOATS == 1
            if (Type == ActorDataType::Float){
                Input->Data = malloc(sizeof(float));
            }
        #endif
        if (Type == ActorDataType::PassThrough){
            Input->Data = new(String);
        }
    }
    void Actor::FormatInteger(String * Str, uint8_t Input, uint8_t LeadingZeros){
        uint8_t LeadingZerosTemp = 0;
        String Number((int)Input);
        //12, 2 -> 012, 
        if(LeadingZeros>Number.length()){
            LeadingZerosTemp = LeadingZeros - Number.length();
            for(uint8_t i=0;i<LeadingZerosTemp;i++){
                *Str = *Str + Defines::CharFalse;
            }
        }
        *Str = *Str + Number;
    }

    bool Actor::IsEnabled(){ return this->Enabled;}
    void Actor::Enable(bool Set){ this->Enabled = Set;}
#endif
