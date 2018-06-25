#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <malloc.h>

#include <switch.h>


extern "C" {
    extern u32 __start__;

    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x100000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void __appInit(void) {
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }
    
    rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }
}

void __appExit(void) {
    /* Cleanup services. */
    fsdevUnmountAll();
    fsExit();
    smExit();
}

static u64 creport_parse_u64(char *s) {
    /* Official creport uses this custom parsing logic... */
    u64 out_val = 0;
    for (unsigned int i = 0; i < 20 && s[i]; i++) {
        if ('0' <= s[i] && s[i] <= '9') {
            out_val *= 10;
            out_val += (s[i] - '0');
        } else {
            break;
        }
    }
    return out_val;
}

int main(int argc, char **argv) {
    /* Validate arguments. */
    if (argc < 2) {
        return 0;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            return 0;
        }
    }
    
    /* Parse arguments. */
    u64 crashed_pid = creport_parse_u64(argv[0]);
    bool has_extra_info = argv[1][0] == '1';
    
    /* TODO: Generate report. */
    (void)(has_extra_info);
    
    if (R_SUCCEEDED(nsdevInitialize())) {
        nsdevTerminateProcess(crashed_pid);
        nsdevExit();
    }
    
    /* TODO: fatalWithType(<report error>, FatalType_ErrorScreen); */
    fatalWithType(0, FatalType_ErrorScreen);
}