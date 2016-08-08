// Consistent protocol for RS232 & UDP integration
//
// This script emulate the Butler XT2 RS232 protocol specification in Programmer. 
// It also provides the same protocol specification to be used over UDP,
// allowing for consistent system integration procdures regardless of the technology
// being used. 
//
// In addition to emulating the existing RS232 protocol from the Butler XT2, this script
// also adds a number of features, such as playing a specific cue within a cuelist,
// which are not supported by the XT2 in standalone mode. 
//
//
int sharedBob = BobAllocate(500);
string message;
int commandProcessed = _false;

RegisterEvent(UdpReceive, OnUdpReceive);
RegisterEvent(SerialPort, OnSerialPort);
Suspend();

function OnUdpReceive(int  nDriverHandle) // UDP frame received.
{	
	commandProcessed = _false;
	
	int drv = nDriverHandle;
	BobSetRange(sharedBob, 0, 500, 0);
	string ip = "";
	int port = -1;
	
	ReceiveFrom(drv, sharedBob, ip, port);
	message = BobGetString(sharedBob, 0, 500);
	routeMessage();
}


function OnSerialPort(int  p0,int  p1,int  p2) // Serial Port - This triggers to events on the serial ports. The meaning of parameters depends on which protocol stack is selected.
{
	processSerialCommand();
}

function processSerialCommand(){
commandProcessed = _false;
	int drv = DriverGetHandle("serial");
	int len;
	message = GetSerialString(drv,len);
	if(len>2){
		routeMessage();	
	} 
	if(len>-1){
		processSerialCommand();
	}
}



function routeMessage(){
	message = strToUpper(message);
	checkMinMessageLength(2);
	
	string command = midstr(message, 0, 2);
	
	if(strcmp(command, "PC") ==0) {cmdPlayCuelist();}
	if(strcmp(command, "PQ") ==0) {cmdPlayCue();}
	if(strcmp(command, "IN") ==0) {cmdSetVmLevel();}
	if(strcmp(command, "ST") ==0) {cmdStopCuelist();}
	if(strcmp(command, "TP") ==0) {cmdTogglePlay();}
	if(strcmp(command, "PP") ==0) {cmdTogglePause();}
	if(strcmp(command, "AF") ==0) {cmdAutoFade();}
	if(strcmp(command, "NX") ==0) {cmdPlayNextMutex();}
	if(strcmp(command, "PX") ==0) {cmdPlayPrevMutex();}
	
	if(!commandProcessed){
		alert("UDP Command was not recognized.\n");
	}
}

function cmdPlayCuelist(){
	int cuelist = QL(getParam(0));
	CuelistStart(cuelist);
	commandProcessed = _true;
}

function cmdPlayCue(){
	int cuelist = QL(getParam(0));
	int cue = Q(getParam(1));
	CuelistGotoCue(cuelist, cue, _fade);
	commandProcessed = _true;
}

function cmdSetVmLevel(){
	int vm = getParam(0);
	int level = getParam(1);
	VersatileMasterStartAutoFade(vm, level, 0);
	commandProcessed = _true;
}

function cmdAutoFade(){
	int vm = getParam(0);
	int level = getParam(1);
	int time = 1000* getParam(2);
	VersatileMasterStartAutoFade(vm, level, time);
	commandProcessed = _true;
}

function cmdStopCuelist(){
	int cuelist = QL(getParam(0));
	if(cuelist==QL(0)){
		CuelistStopAll();
	} 
	else {
		CuelistStop(cuelist);
	}
	commandProcessed = _true;
}

function cmdTogglePlay(){
	int cuelist = QL(getParam(0));
	int currentState = CueGetCurrent(cuelist);
	
	if(currentState<0){
		CuelistStart(cuelist);
	} 
	else {
		CuelistStop(cuelist);
	}
	commandProcessed = _true;
}


function cmdPlayNextMutex(){
	int mutex = getParam(0);
	playNextMutex(mutex);

	commandProcessed = _true;
}

function cmdPlayPrevMutex(){
	int mutex = getParam(0);
	playPrevMutex(mutex);

	commandProcessed = _true;
}

function cmdTogglePause(){
	int cuelist = QL(getParam(0));
	int currentStatus = CuelistIsPaused(cuelist);
	
	CuelistPause(cuelist);
	
	
	if(currentStatus == _true){
		CuelistStart(cuelist);
	} 
	else {
		CuelistPause(cuelist);
	}
	
	commandProcessed = _true;
}





function playNextMutex(int mutex){
	int ql = getNextMutexCuelist(mutex);
	CuelistStart(ql);
}

function playPrevMutex(int mutex){
	int ql = getPrevMutexCuelist(mutex);
	CuelistStart(ql);
}


function getNextMutexCuelist(int mutex){
	int i;
	int qlMutex;
	
	int cuelist = CuelistMutexGetStatus(mutex);
	
	for (i=cuelist+1; i<=100; i++)
	{
		if(cuelistExists(i)){
			
			qlMutex = CuelistGetProperty(i, "MutualExcludeGroup");
			if(mutex == qlMutex){
				return i;
			}
		}
	}	
	
	for (i=0; i<=cuelist-1; i++)
	{
	
		if(cuelistExists(i)){
		
			qlMutex = CuelistGetProperty(i, "MutualExcludeGroup");
			if(mutex == qlMutex){
				return i;
			}
		}
	}
	
	return -1;
}


function getPrevMutexCuelist(int mutex){
	int i;
	int qlMutex;
	
	int cuelist = CuelistMutexGetStatus(mutex);
	
	for (i=cuelist-1; i>=0; i--)
	{
		if(cuelistExists(i)){
			
			qlMutex = CuelistGetProperty(i, "MutualExcludeGroup");
			if(mutex == qlMutex){
				return i;
			}
		}
	}	
	
	for (i=100; i>=cuelist+1; i--)
	{
	
		if(cuelistExists(i)){
		
			qlMutex = CuelistGetProperty(i, "MutualExcludeGroup");
			if(mutex == qlMutex){
				return i;
			}
		}
	}
	
	return -1;
}



function cuelistExists(int cuelist){
	return (CueGetCount(cuelist)>0);
}


function getParam(int paramNum){
	int start = 2 + (paramNum * 3);
	int count = 3;
	
	int minMessageLength = start + count;
	checkMinMessageLength(minMessageLength);
	
	string paramStr = midstr(message, start, count);
	return val(paramStr);
}

function checkMinMessageLength(int length){
	if(strlen(message)<length){
		alert("Invalid UDP Message received: ERR_LENGTH %d, %d\n", strlen(message), length);
		bailOut();
	}
}

function bailOut(){
	Call("unifiedControlProtocol", 0, 0, 0);
	exit();
}

 