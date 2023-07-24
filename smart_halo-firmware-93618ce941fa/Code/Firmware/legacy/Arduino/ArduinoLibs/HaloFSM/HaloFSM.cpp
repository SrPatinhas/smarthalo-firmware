#include "HaloFSM.h"

// TIMERS (ms)
#define STILLNESS_TIMEOUT 	60000//120000
#define ALARM_TIMEOUT 		10000
#define LOCK_TIMEOUT 		60000//300000
#define IDLE_TIMEOUT 		15000
#define WAKEUP_TIMEOUT 		60000//600000

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

HaloFSM::HaloFSM()
: CurrentPowerState(SLEEP), NextPowerState(SLEEP), CurrentDisplayMode(TBT), tStill(0),
  tAlarm(0), tLock(0), tIdle(0), tStillOn(false), tAlarmOn(false), tLockOn(false), tIdleOn(false)
{
	tWup = millis();
	tWupOn = true;
}

HaloFSM::PSTATE HaloFSM::getPowerState()
{
	return CurrentPowerState;
}

HaloFSM::DMODE HaloFSM::getDisplayMode()
{
	return CurrentDisplayMode;
}

void HaloFSM::sendToNap()
{
	// Go to NAP only if paired to phone
	if(CurrentPowerState == PAIRED || CurrentPowerState == RIDING)
		NextPowerState = NAP, tWup = millis(); tWupOn = true;
}
		
void HaloFSM::forceWakeUp()
{
	// Wakeup if in NAP
	if(CurrentPowerState == NAP)
		NextPowerState = RIDING;
}
		
void HaloFSM::motionDetected(bool motion)
{	
	// All transitions based on motion/stillness
	switch(CurrentPowerState)
	{
		case SLEEP:
			if(motion)
				tWup = 0, tWupOn = false, NextPowerState = PARKED, tAlarm = millis(), tAlarmOn = true;	
			break;			
		case PARKED:
			// Stop-Reset Alarm timer on stillness and wakeup on motion
			if(!motion)
				tAlarm = 0, tAlarmOn = false;
			else
				tIdle = 0, tIdleOn = false;
			
			// Start Alarm timer on motion and wakeup on stillness
			if(motion && tAlarm == 0) 
				tAlarm = millis(), tAlarmOn = true;			
			else if(!motion && tIdle == 0)
				tIdle = millis(), tIdleOn = true;				
			break;
		case ALARM:
			if(motion)
				tStill = 0; tStillOn = false;
			
			if(!motion && tStill == 0)
				tStill = millis(), tStillOn = true;			
			break;
		case UNLOCKED:
			if(!motion && tLock == 0)
				tLock = millis(), tLockOn = true;
			else if(motion) tLock = 0, NextPowerState = SA_RIDE, tLockOn = false;
			break;
		case PAIRED:
			if(!motion && tLock == 0)
				tLock = millis(), tLockOn = true;
			if(motion) tLock = 0, NextPowerState = RIDING, tLockOn = false;
			break;
		case SA_RIDE:
		case RIDING:
			if(motion)
				tStill =0, tStillOn = false;
			
			if(!motion && tStill == 0)
				tStill = millis(), tStillOn = true;
			break;
		default:			
			break;
	};
}

void HaloFSM::codeEntered()
{	
	if(CurrentPowerState == ALARM || CurrentPowerState == PARKED)
	{
		Serial.println(""); Serial.print("Code entered"), NextPowerState = UNLOCKED;	
		tStill = 0, tStillOn = false; // If CurrentPowerState = ALARM
		tIdle = 0, tIdleOn = false; // If CurrentPowerState = PARKED
		tLock = millis(), tLockOn = true;
	}
}

void HaloFSM::BTConnection(bool connected)
{
	if(connected)
	{	
		Serial.println(""); Serial.print("BT Connection established");
	}
	else
	{
		Serial.println(""); Serial.print("BT Connection lost");
	}
	
	switch(CurrentPowerState)
	{
		case PARKED:
			if(connected) 
			{
				NextPowerState = PAIRED;
				tIdle = 0, tIdleOn = false;
				tLock = millis(), tLockOn = true;
			}
			break;
		case UNLOCKED:
			if(connected) NextPowerState = PAIRED;
			break;				
		case PAIRED:
			if(!connected) NextPowerState = UNLOCKED;
			break;
		case SA_RIDE:
			if(connected) NextPowerState = RIDING;
			break;
		case RIDING:
			if(!connected) NextPowerState = SA_RIDE;
			break;
		default:
			break;
	};
}

// Timer events
void HaloFSM::stillnessTimeout()
{
	Serial.println(""); Serial.print("Stillness timeout");
	switch(CurrentPowerState)
	{		
		case SA_RIDE:
			NextPowerState = UNLOCKED;
			tLock = millis(), tLockOn = true;
			break;
		case RIDING:
			NextPowerState = PAIRED;
			tLock = millis(), tLockOn = true;
			break;
		case ALARM:
			NextPowerState = PARKED;
			tIdle = millis(), tIdleOn = true;
			break;
		default:
			break;
	};
}

void HaloFSM::alarmTimeout()
{
	if(CurrentPowerState == PARKED)
		Serial.println(""), Serial.print("Alarm timeout"), NextPowerState = ALARM;
			
}

void HaloFSM::lockTimeout()
{
	if(CurrentPowerState == UNLOCKED || CurrentPowerState == PAIRED)
	{
		Serial.println(""), Serial.print("Lock timeout");
		NextPowerState = PARKED;		
		tIdle = millis(), tIdleOn = true;
	}
}

void HaloFSM::idleTimeout()
{
	if(CurrentPowerState == PARKED)
	{
		Serial.println(""), Serial.print("Idle timeout");
		NextPowerState = SLEEP;
		tWup=millis(), tWupOn = true;
	}
}

void HaloFSM::wakeUpTimeout()
{
	Serial.println(""), Serial.print("Periodic Wake Up");
	
	if(CurrentPowerState == SLEEP)
	{
		NextPowerState = PARKED;
		tIdle = millis(), tIdleOn = true;		
	}
	else if(CurrentPowerState == NAP)
	{
		NextPowerState = RIDING;
	}
}

void HaloFSM::tapDetected()
{
	switch(CurrentDisplayMode)
	{
		case TBT:
			CurrentDisplayMode = CROW;
			break;
		case CROW:
			CurrentDisplayMode = GC;
			break;
		case GC:
			CurrentDisplayMode = TBT;
			break;
		default:
			CurrentDisplayMode = TBT;
			break;
	};
}

void HaloFSM::updateTimers()
{
	unsigned long now;	
	
	// Check timeouts
	now = millis();
	Serial.println("");
	Serial.print("tStill: "); Serial.print(tStillOn?(now-tStill):tStill);
	
	if(tStillOn && ((now - tStill) >= STILLNESS_TIMEOUT))
	{
		tStill = 0;
		tStillOn = false;
		stillnessTimeout();
	}
	
	now = millis();
	Serial.print(" tAlarm: "); Serial.print(tAlarmOn?(now-tAlarm):tAlarm);
	
	if(tAlarmOn && ((now - tAlarm) >= ALARM_TIMEOUT))
	{
		tAlarm = 0;
		tAlarmOn = false;
		alarmTimeout();
	}
	
	now = millis();
	Serial.print(" tLock: "); Serial.print(tLockOn?(now-tLock):tLock);
	
	if(tLockOn && ((now - tLock) >= LOCK_TIMEOUT))
	{
		tLock = 0;
		tLockOn = false;
		lockTimeout();
	}
	
	now = millis();
	Serial.print(" tIdle: "); Serial.print(tIdleOn?(now-tIdle):tIdle);
	
	if(tIdleOn && ((now - tIdle) >= IDLE_TIMEOUT))
	{
		tIdle = 0;
		tIdleOn = false;
		idleTimeout();
	}
	
	now = millis();
	Serial.print(" tWup: "); Serial.print(tWupOn?(now-tWup):tWup);
	
	if(tWupOn && ((now - tWup) >= WAKEUP_TIMEOUT))
	{
		tWup = 0;
		tWupOn = false;
		wakeUpTimeout();
	}
}

void HaloFSM::processPowerState()
{
	// Change State
	CurrentPowerState = NextPowerState;
	
	// Process transition
	switch(CurrentPowerState)
	{		
		case PARKED:
			// TURN ON EVERYTHING EXCEPT LED AND DRIVERS
			break;
		case UNLOCKED:
			// TURN ON EVERYTHING EXCEPT LED AND DRIVERS
			break;			
		case PAIRED:
			// TURN ON EVERYTHING
			break;
		case SA_RIDE:
			// TURN ON EVERYTHING EXCEPT LED AND DRIVERS
			break;
		case RIDING:
			// TURN ON EVERYTHING
			break;
		case NAP:
			// TURN OFF EVERYTHING EXCEPT BT
			break;	
		case ALARM:
			// TURN ON EVERYTHING AND PIEZO
			break;
		case SLEEP:			
		default:
			// TURN OFF EVERYTHING EXCEPT ACCELEROMETER
			break;
	};
}

const char * HaloFSM::getPowerStateStr()
{
	switch(CurrentPowerState)
	{
		case SLEEP:
			return "SLEEP";
			break;
		case PARKED:
			return "PARKED";
			break;
		case UNLOCKED:
			return "UNLOCKED";
			break;			
		case PAIRED:
			return "PAIRED";
			break;
		case SA_RIDE:
			return "SA_RIDE";
			break;
		case RIDING:
			return "RIDING";
			break;
		case NAP:
			return "NAP";
			break;	
		case ALARM:
			return "ALARM";
			break;
		default:
			return "UNKNOWN";
			break;
	};
}

const char * HaloFSM::getDisplayModeStr()
{
	switch(CurrentDisplayMode)
	{
		case TBT:
			return "TBT";
			break;
		case CROW:
			return "CROW";
			break;
		case GC:
			return "GC";
			break;
		default:
			return "UNKNOWN";
			break;
	};
}