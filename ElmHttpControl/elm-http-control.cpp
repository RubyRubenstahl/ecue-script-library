// ELM HTTP Control
// -----------------------------------------------------------
// Syncs ELM stage media content & intensity level with
// programmer cuelists
// -----------------------------------------------------------
//
// Setup instructions
//
// 1. Enable the HTTP driver in ELM
// 2. Add a TCP driver to your show file with an alias of "elm" and the
//    IP address & Port set to the match the settings within ELM
// 3. Configure this script to start at initialization
// 4. Create a cuelist for each stage in the form of "elm: <STAGE_NAME>"
//    for example, for Stage01, your cuelist name should be "elm: Stage01"
// 5. Add a dummy cue for each media item in your elm stage.
//
// This script will use the naming scheme of the cuelists to determine which
// cuelists to sync with ELM.
//
// When a cue plays, the media item with the corresponding number will be played in ELM.
// The cuelist fade time will be read from the programmer list and will be used as the
// transition time within elm.
//
// The cuelist intensity level will also be synced with the stage intensity in ELM, allowing
// for action pad control
//
// Note: This script uses the TCP driver to manually send HTTP packets as the headers cannot
// be set using the built-in HTTP driver and specfic headers are required for the ELM
// functionality to work properly.



// ----------------- USER EDITABLE VARIABLES ------------------------

// The alias name of the TCP driver to use
string DRIVER_ALIAS = "elm"

// Number of cuelists to scan. Higher numbers could lead to 
// performance problems on processor intensive shows. 
int CUELIST_COUNT = 256;

// ----------------- DO NOT EDIT BELOW THIS POINT ------------------------

int elmHttp = DriverGetHandle(DRIVER_ALIAS);

int packet = BobAllocate(15000);
int headerCursor = 0;
int cursor = 0;

int prevCues[CUELIST_COUNT];
int prevCueLevels[CUELIST_COUNT];
string POST = "POST";

function newPacket()
{
    headerCursor = 0;
}

// Convert an integer to a floating point integer string
// val = 1000x the value of the desired output
string floatStr = "";
function toFloatStr(int val)
{
    string intStr = format("%04d", val);
    int index = strlen(intStr) - 3;
    floatStr = strInsert(intStr, index, ".");
}

// Push an HTTP header to the packet
function pushHeader(string value)
{
    string header = format("%s\n", value);
    BobSetString(packet, headerCursor, strlen(header) + 1, header);
    headerCursor += strlen(header);
}

// Add the final newline to signify the end of the HTTP header
function closepacket()
{
    BobSetString(packet, headerCursor, 2, "\n");
    headerCursor += 1;
}

// Add the request line to the beginning of the HTTP packet
function pushRequest(string method, string path)
{
    string header = format("%s %s HTTP/1.1\n", method, path);
    BobSetString(packet, headerCursor, strlen(header) + 1, header);
    headerCursor += strlen(header);
}

// Build and send an HTTP packet
// method = HTTP method ("GET", "POST", "PUT", "REMOVE");
// path = relative URL resource path (eg "/elm/stages/Stage01/live?media=1")
function sendRequest(string method, string path)
{
    newPacket();
    pushRequest(method, path);
    pushHeader("Accept: */*");
    pushHeader("Host: 127.0.0.1:8080");
    pushHeader("Content-Length: 0");
    pushHeader("Connection: keep-alive");
    pushHeader("User-Agent: Programmer/7.0");
    closepacket();

    TcpSend(elmHttp, packet, headerCursor);
}

// Returns true if cuelist starts with "elm:", signifying
// that the cuelist should be tracked by this script
function isElmCuelist(string qlName)
{
    string prefix = midstr(qlName, 0, 4);
    return strcmp(prefix, "elm:") == 0;
}

// Check for changes to a tracked cuelist and sync if any changes
// have occured
function updateElmStage(int ql, string stageName)
{
    string path;
    int currentCue = CueGetCurrent(ql) + 1;
    int fadtimeMs;

    if (currentCue != prevCues[ql])
    {
        prevCues[ql] = currentCue;
        int fadeTimeMs = CueGetProperty(ql, currentCue - 1, "InFadeTime");
        toFloatStr(fadeTimeMs);
        path = format("/elm/stages/%s/live?media=%d&transitionDuration=%s", stageName, currentCue, floatStr);
        sendRequest(POST, path);
    }

    int currentLevel = CuelistSubMasterGetValue(ql);

    if (currentLevel != prevCueLevels[ql])
    {
        prevCueLevels[ql] = currentLevel;
        toFloatStr(currentLevel * 1000 / 4096);
        path = format("/elm/stages/%s/live?intensity=%s", stageName, floatStr);
        sendRequest(POST, path);
    }
}

// Loop through all cuelists and run update on
// any cuelist that has an "elm:" prefix
function updatePlayState()
{
    int i;
    string qlName;
    string prefix;
    string stageName;
    for (i = 0; i < CUELIST_COUNT; i++)
    {
        qlName = CuelistGetName(i);
        if (isElmCuelist(qlName))
        {
            stageName = strTrim(midstr(qlName, 4, strlen(qlName - 4)), " ");
            updateElmStage(i, stageName);
        }
    }
}

// Run the change detection and updates every
// frame
function OnFrame()
{
    updatePlayState();
}

RegisterEvent(Frame, OnFrame);
Suspend();
