# DLogs

Convenient tool to make logs or debug prints in any project.

( Working and using in others projects project but in my opinion code is young compared with lates projects. Actual project with my current code-style: `DPeerNode`.
So projects with "young code" in plan to rework )


## Getting start
To use DLogs tools you need own `DLogs::DLogsContext`. This logs context is core of all DLogs tools. 
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

int DSynapse::DSynapseInitLogContext(const char *stream_name)
{
    DLOGS_INIT_DEFAULT_CONTEXT(stream_name);
    
    // 
    log_context.set_log_level(0);
    log_context.set_lvl_cmp_callback(DLogs::default_lvl_cmp__more_oe);
    return 0;
}
```


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
# Tune own log context
## Parse logs to file(s)
## Parse logs to default printf
## Parse logs to own callback
## Log level
## Tune log header
