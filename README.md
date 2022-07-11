# DLogs

Convenient tool to make logs or debug prints in any project.

( Working and using in others projects project but in my opinion code is young compared with lates projects. Actual project with my current code-style: `DPeerNode`.
So projects with "young code" in plan to rework )

## Terms
```
log-call - Call of DLogs function to make log. Log context can ignore log-call.
log-line - Text implementation of log without header

```

## Getting start
To use DLogs tools you need own `DLogs::DLogsContext` instance. This logs context is core of all DLogs tools. 
Create own context or several contexts to tune each with needed settings.

### In my projects I use fast way to create and init logs context:

> ProjectGlobalHeader.h:

```
namespace YourProjectLogsNamespace {
    extern DLogs::DLogsContext log_context; // strict: to use fast macros this global variable should has this name (log_context)
    extern DLogs::DLogsContextInitializator logsInit; // fast log context initilizator. This could has any name
}
```
> ProjectGlobalHeader.cpp:

```

#include "ProjectGlobalHeader.h"

DLogs::DLogsContext YourProjectLogsNamespace::log_context; // global log context definition

/*
This macro create unsuable global object with type DLogs::DLogsContextInitializator with name Initializator. Constructor of this class
initializate log context with default settings.
*/
DLOGS_INIT_GLOBAL_CONTEXT("ProjectName", YourProjectLogsNamespace::logsInit); 
```

### Another way to start:

> ProjectGlobalHeader.h:
```
namespace YourProjectLogsNamespace {
    extern DLogs::DLogsContext log_context; 
    int ProjectGlobalLogsContextStartFunction( const char *project_name );
}
```

> ProjectGlobalHeader.cpp:

```
#include "ProjectGlobalHeader.h"

DLogs::DLogsContext YourProjectLogsNamespace::log_context; // global log context definition

int ProjectGlobalLogsContextStartFunction(const char *stream_name)
{
    // Unnecessary:
    DLOGS_INIT_DEFAULT_CONTEXT(stream_name);
    
    // Custom settings:
    log_context.set_log_level(0);
    log_context.set_lvl_cmp_callback(DLogs::default_lvl_cmp__more_oe);
    return 0;
}
```
Macro `DLOGS_INIT_DEFAULT_CONTEXT` does 3 things:
- Set stream name in log context ( About stream name see `Log header` part )
- Set default header settings
- Create all default messages for fast macros ( About messages see `Messages` part )

## C-Style macros
To make fast log in some function use fast macro with C-style text input.

Default macros by DLogs:
```
DL_BADPOINTER(level, ...)
DL_BADVALUE(level, ...) 
DL_FUNCFAIL(level, ...) 
DL_BADALLOC(level, ...) 
DL_ERROR(level, ...) 
DL_WARNING(level, ...) 
DL_INFO(level, ...) 
```
- All macros do same thing but with different prefix. 
- All macros use global log context which should be available for macro call place

## C++ std::cout/std::cin objects
`DLogs::DLogsMaster`

# Tune own log context
## Parse logs to file(s)
You can add file parsing to you log context. To enable, call: `DLogsContext::parse_file(true)`.
Interface:
```
    void DLogsContext::file_add   (const char *path, const char *key);
    void DLogsContext::file_lock  (const char *key);
    void DLogsContext::file_unlock(const char *key);
    void DLogsContext::file_remove(const char *key);
    
    // Parse log-calls only with this levels (white-list):
    void DLogsContext::file_add_level(const char *key, int level); 
    void DLogsContext::file_remove_level(const char *key, int level);
    
    typedef int (*PARSE_FILE_TOOL)(FILE*, const char *line);
    void DLogsContext::parse_set_file_tool      (PARSE_FILE_TOOL tool);
    
    // By default:
    int DLogs::default_file_parse(FILE *f, const char *line)
    {
        fwrite(line, 1, strlen(line), f);
        return 0;
    }
```
## Parse logs to console
To enable, call: `DLogsContext::parse_console(true)`.
Interface:
```
    typedef int (*PARSE_CONSOLE_TOOL)(const char *line);
    void DLogsContext::parse_set_console_tool      (PARSE_CONSOLE_TOOL tool);
``` 
Default behaivor:
```
    int DLogs::default_console_parse(const char *line)
    {
        printf("%s",line);
        return 0;
    }
```

## Parse logs to own callback
To enable, call: `DLogsContext::parse_special(true)`.
Interface:
```
    typedef int (*PARSE_SPECIAL_TOOL)(int level, void*, const char *line);
    void DLogsContext::parse_set_special_tool      (PARSE_SPECIAL_TOOL tool);
``` 


## Log level

Each log receive log level (`int`) as argument.
Log level mechanic provide possibility to separate parse-behaivor for logs with different log-level values.
> Log context use global log-level:

You can set global log-level to your log context. 
Each time you use log-call - log context compare log-level of your log-call with global log-level
to make a decision of parsing this log-line.
You can set own compare callback to log context. Default or create own. Default callbacks:
```
    typedef bool (*LOG_LEVEL_CMP_CALLBACK)(int base_level, int local_level);
    
    bool default_lvl_cmp__more(int base_level, int local_level);
    bool default_lvl_cmp__more_oe(int base_level, int local_level);
    bool default_lvl_cmp__less(int base_level, int local_level);
    bool default_lvl_cmp__less_oe(int base_level, int local_level);
    bool default_lvl_cmp__equal(int base_level, int local_level);
```
Use ` void DLogs::DLogsContext::set_lvl_cmp_callback(LOG_LEVEL_CMP_CALLBACK c) ` to change compare callback. Global log-level
don't change parse-behaivor, only make a decision about igonring this log-call.



> Log context use map of parse-behaivors ( key ~ level, value ~ behaivor ):

You can set custom sub-context for different log-levels. (Mechanic in plan).

> Example:

```
// Global log-level example:
int ProjectGlobalLogsContextStartFunction(const char *stream_name) {
    DLOGS_INIT_DEFAULT_CONTEXT(stream_name);
    
    log_context.set_log_level(0);
    log_context.set_lvl_cmp_callback(DLogs::default_lvl_cmp__more);
    return 0;
}
bool test_function( int n ) {
    
    if( n < 1 ) {
        DL_BADVALUE( 2, "n: [%d]", n ); // log-level: 2
        return false;
    }
    while( n-- ) {
        // do something...
        DL_INFO( 1, "Some info about iteration..." ); // log-level: 1
    }
    return true;
}
```
With compare behaivor `more` ( log_level > global_log_level) and global log-level `0` both log-calls will be parse.
But if you compile code with: ``` log_context.set_log_level(1); ```: `DL_INFO( 1, "Some info about iteration..." );` log-call will be ingored.
If you compile code with ``` log_context.set_log_level(2); ```: both log-calls will be ingored.

```
// Custom log-levels mechanic in process and will looks like:
int ProjectGlobalLogsContextStartFunction(const char *stream_name) {
    DLOGS_INIT_DEFAULT_CONTEXT(stream_name);
   
   // Set different behaivors:
   
   auto LogLevelDependedBehaivorDescriptor0 = 
   log_context.set_custom_lvl( 0 );
   LogLevelDependedBehaivorDescriptor0.parse_console(true);
   
   log_context.set_custom_lvl( 1 );
   LogLevelDependedBehaivorDescriptor1 = log_context.custom_lvl_desc( 1 );
   
   LogLevelDependedBehaivorDescriptor1.parse_console(false);
   LogLevelDependedBehaivorDescriptor1.parse_file(true);
   LogLevelDependedBehaivorDescriptor1.file_add( "LogFile.txt" );
    
    return 0;
}
bool test_function( int n ) {
    
    if( n < 1 ) {
        DL_BADVALUE( 1, "n: [%d]", n ); // log-level: 1
        return false;
    }
    while( n-- ) {
        // do something...
        DL_INFO( 0, "Some info about iteration..." ); // log-level: 0
    }
    return true;
}
```
Parsing log-lines with level `0` only to console and parsing log-lines with level `1` only to file.




## Log header

Log header is prefix of each log line. DLogs header consist of header blocks and each blocks can be hidden.
DLogs provide 7 different block for header:
- Date - current date of log
- Local time - current time of log
- Function name - name of function contained this log
- Log number - value of logs counter
- Run time - time since program start
- Stream name - log context logo
- Level - log input level

Interface:
```
    struct header_state
    {
        bool cdate;
        bool fname;
        bool ltime;
        bool mnumb;
        bool rtime;
        bool sname;
        bool level;
    };
        void DLogsContext::header_set    (const header_state &hs);
        void DLogsContext::header_set_all(
                                bool date   = false,
                                bool fname  = false,
                                bool ltime  = false,
                                bool rtime  = false,
                                bool mesnum = false,
                                bool sname  = false,
                                bool level = false
                            );


        DLogsContext *DLogsContext::header_set_message(const char *message);
        DLogsContext *DLogsContext::header_set_message(int message_key);
        int DLogsContext::add_message(const char *m, int key);
```

You can also disable the header at all.

## Messages
Message just is block of header which contain any text. You can change message each log-call without changhing log-context.


## Thread safe logs
Thread safe logs parsing in plan...
