#ifndef HALOFSM_H_
#define HALOFSM_H_


class HaloFSM
{
	public:
		
		enum PSTATE {SLEEP = 0, PARKED, ALARM, UNLOCKED, PAIRED, SA_RIDE, RIDING, NAP};
		enum DMODE 	{TBT = 0, CROW, GC};
		
		// Ctor/Dtor
		HaloFSM();
		~HaloFSM() {};
		
		// Accessors
		PSTATE getPowerState();
		DMODE getDisplayMode();
		
		// StringAccessors
		const char * getPowerStateStr();
		const char * getDisplayModeStr();
		
		// Control signals
		void sendToNap();
		void forceWakeUp();
		void tapDetected();
		
		// Events
		void motionDetected(bool motion=true);
		void codeEntered();
		void BTConnection(bool connected=true);
		
		// These must be called	in the main loop	
		void updateTimers();
		void processPowerState();
				
	private:
		
		// Timer events
		void stillnessTimeout();
		void alarmTimeout();
		void lockTimeout();
		void idleTimeout();
		void wakeUpTimeout();
		
	private:	
	
		PSTATE CurrentPowerState, NextPowerState;
		DMODE CurrentDisplayMode;
		
		unsigned long	tStill;
		unsigned long	tAlarm;
		unsigned long	tLock;
		unsigned long	tIdle;
		unsigned long	tWup;
		bool tStillOn, tAlarmOn, tLockOn, tIdleOn, tWupOn;
};

#endif // HALOFSM_H_