#ifndef DLOGS_H
#define DLOGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include <cstddef>
#include <iostream>
#include <stddef.h>
#include <map>

#include <DArray.h>



#define DLOGS_DEFAULT_BUFFER_SIZE (1024)
#define DLOGS_MIN_BUFFER_SIZE (64)
#define DLOGS_MAX_BYTES_PUT_TO_BUFFER (DLOGS_DEFAULT_BUFFER_SIZE * 4)
#define DLOGS_LOCAL_BUFFER_SIZE 128
#define D_FUNC_NAME __func__
#define VN(var) " "##var": " << var
namespace DLogs
{
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
    extern char local_buffer[DLOGS_LOCAL_BUFFER_SIZE];


    struct DLogsFile
    {
        FILE *target;
        std::string key;
        std::string path;
        bool is_available;
        DArray<int> levels;
    };


    typedef int (*PARSE_CONSOLE_TOOL)(const char *line);
    typedef int (*PARSE_FILE_TOOL)(FILE*, const char *line);
    typedef int (*PARSE_SPECIAL_TOOL)(int level, void*, const char *line);
    typedef bool (*LOG_LEVEL_CMP_CALLBACK)(int base_level, int local_level);

    bool default_lvl_cmp__more(int base_level, int local_level);
    bool default_lvl_cmp__more_oe(int base_level, int local_level);
    bool default_lvl_cmp__less(int base_level, int local_level);
    bool default_lvl_cmp__less_oe(int base_level, int local_level);
    bool default_lvl_cmp__equal(int base_level, int local_level);

    int default_console_parse(const char *line);
    int default_file_parse(FILE *f, const char *line);

    class OpDLFileParser
    {
    public:
        OpDLFileParser(PARSE_FILE_TOOL t, const char *d, int l) : tool(t), data(d), level(l) {}
        void operator()(const DLogsFile &file) {
            if(file.is_available && file.target && (level == 0 || file.levels.count(level)))
                tool(file.target, data);
        }

    private:
        PARSE_FILE_TOOL tool;
        const char *data;
        int level;
    };



    class DLogsMaster;
    class DLogsContext
    {
    public:
        DLogsContext();
        DLogsContext(const char *stream_name);
        ~DLogsContext();


        DLogsContext(const DLogsContext &c);
//        DLogsContext& operator=(const DLogsContext &c);

        //----------parse
        void parse_console(bool state);
        void parse_file   (bool state);
        void parse_special(bool state);

        void set_lvl_cmp_callback(LOG_LEVEL_CMP_CALLBACK c);

        void parse_set_console_tool      (PARSE_CONSOLE_TOOL tool);
        void parse_set_file_tool         (PARSE_FILE_TOOL tool);
        void parse_set_special_tool      (PARSE_SPECIAL_TOOL tool);
        void set_special_data(void *data);
        //----------file
        void file_add   (const char *path, const char *key);
        void file_lock  (const char *key);
        void file_unlock(const char *key);
        void file_remove(const char *key);
        void file_add_level(const char *key, int level);
        void file_remove_level(const char *key, int level);


        bool file_is_available(const char *key);

        void header_set    (const header_state &hs);
        void header_set_all(bool date   = false,
                            bool fname  = false,
                            bool ltime  = false,
                            bool rtime  = false,
                            bool mesnum = false,
                            bool sname  = false,
                            bool level = false);


        DLogsContext *header_set_message(const char *message);
        DLogsContext *header_set_message(int message_key);
        int add_message(const char *m, int key);
        //----------separator
        void separator_set_size(int size);
        void separator_set_symb(char symb);
        //----------precision
        void precision_float (unsigned int p);
        void precision_double(unsigned int p);


        void add_header(int level, const char *caller_name, int messageNumber = -1);
        void print_new_line(int level);
        void add_new_line(int level);
        void print_space(int level);
        void add_space(int level);

        size_t set_buffer_size(int s);
        size_t expand_buffer(int s);


        void set_log_level(int l);
    public:

        void init_all();
        const char *get_float_format();
        const char *get_double_format();


        void out(int level, const char *o);
        size_t flush(int level);
        size_t flush(int level, const char *line);
        int put_to_buffer(int level, const char *line);
        template <class T>
        int put_to_buffer(int level, const char *format, T a)
        {
            if(format)
            {
                if(buffer_current_level != level)
                {
                    flush(buffer_current_level);
                    buffer_current_level = level;
                }
                size_t l = snprintf(NULL, 0, format, a);
                size_t free_space = buffer.size() - buffer_pos;
                if(l > free_space)
                {
                    flush(level);
                    free_space = buffer.size();
                }

                l = snprintf(buffer.begin() + buffer_pos, free_space, format, a);
                if(l>0)
                    buffer_pos += l;
            }
        }

        int parse_file_state(FILE *f, char state);

        //----
        char format[4];


        DArray<DLogsFile> files;

        bool is_console_available;
        bool is_file_available;
        bool is_special_available;

        PARSE_CONSOLE_TOOL     c_print_callback;
        PARSE_FILE_TOOL        f_print_callback;
        PARSE_SPECIAL_TOOL     s_print_callback;
        void *special_opaque;
        LOG_LEVEL_CMP_CALLBACK lvl_cmp;

        header_state header;
        int log_level;

        unsigned int message_counter;

        std::string stream_name;
        std::string header_message;
        std::string separator;
        std::map<int, std::string> messages;

        int float_precision;
        int double_precision;


//        char *buffer;
//        size_t buffer_size;

        DArray<char> buffer;
        int buffer_pos;
        int buffer_current_level;

        bool is_expandable_buffer;
        bool is_restorable_buffer;
        int buffer_max_expanded_size;



        template<class ... Args>
        friend void dlogs_base(int level, const char *caller_name, DLogsContext *context, const char* fmt, Args ... a);
        friend class DLogsMaster;


        template<class ... Args>
        void dlogs_base_inner(int level, const char *caller_name, int messageNumber, const char* fmt, Args ... a)
        {
//            size_t f = 0;
//            f = flush(level);
//            if(f)
//                print_new_line(level);

//            int l = snprintf(NULL, 0, fmt, a...);
//            int buffer_old_size = buffer.size();
//            if(l > buffer.size())
//            {
//                buffer_old_size = expand_buffer(l);
//            }
//            add_header(level, caller_name, messageNumber);
//            flush(level);
//            l = snprintf(buffer.begin(), buffer.size(), fmt, a...);
//            buffer_pos += l;
//            f = flush(level);
//            if(f)
//                print_new_line(level);

//            if(is_restorable_buffer)
//            {
//                set_buffer_size(buffer_old_size);
//            }

            copy_mem(buffer.begin(), fmt, strlen(fmt));
//            int l = snprintf(buffer.begin(), buffer.size(), fmt, a...);
            buffer_pos += strlen(fmt);
            if( flush(level) ) {
                print_new_line(level);
            }

        }



        friend void dlogs_separator(int level, DLogsContext *context);
        void dlogs_separator_inner(int level);
    };

    class DLogsMaster
    {
    public:
        DLogsMaster(DLogsContext *c, bool auto_nl = false);
        DLogsMaster(DLogsContext *c, bool temporary, const char *caller_name);
        ~DLogsMaster();

        int setLevel(int l);

        DLogsMaster& operator()(int l);
        DLogsMaster& operator()(int l, const char *caller_name);


        DLogsMaster &operator <<(int v);
        DLogsMaster &operator <<(const char*   v);
        DLogsMaster &operator <<(float  v);
        DLogsMaster &operator <<(double v);
        DLogsMaster &operator <<(bool   v);
        DLogsMaster &operator <<(long   v);
        DLogsMaster &operator <<(char   v);
        DLogsMaster &operator <<(FILE*);
        template <class T>
        DLogsMaster &operator <<(T* p)
        {
            context->put_to_buffer(level, "%p", p);
            context->add_space(level);
            return *this;
        }

        DLogsMaster &operator <<(std::nullptr_t);
        DLogsMaster &operator <<(DLogsMaster&);
    private:
        DLogsContext *context;
        bool auto_n;
        int level;
    };


    template <class ... Args>
    void dlogs_base(int level, const char *caller_name, int messageNumber, DLogsContext *context, const char* fmt, Args ... a)
    {
        context->dlogs_base_inner(level, caller_name, messageNumber, fmt, a...);
    }

    /*
    template <class ... Args>
    void dlogs_base(int level, const char *caller_name, int messageNumber, DLogsContext *context, const char* fmt, Args ... a)
    {
        size_t f = 0;
        context->flush(level);
        if(f)
            context->print_new_line(level);

        int l = snprintf(NULL, 0, fmt, a...);
        int buffer_old_size = context->buffer.size();
        if(l > context->buffer.size())
        {
            buffer_old_size = context->expand_buffer(l);
        }
        context->add_header(level, caller_name, messageNumber);
        context->flush(level);
        l = snprintf(context->buffer.begin(), context->buffer.size(), fmt, a...);
        context->buffer_pos += l;
        f = context->flush(level);
        if(f)
            context->print_new_line(level);

        if(context->is_restorable_buffer)
        {
            context->set_buffer_size(buffer_old_size);
        }
    }
    */


    void dlogs_separator(int level, DLogsContext *context);


}
#define check___
#define DLOGS_MESSAGE_BADPOINTER__KEY (1001)
#define DLOGS_MESSAGE_BADVALUE__KEY (1002)
#define DLOGS_MESSAGE_FUNCFAIL__KEY (1003)
#define DLOGS_MESSAGE_BADALLOC__KEY (1004)
#define DLOGS_MESSAGE_ERROR__KEY (1005)
#define DLOGS_MESSAGE_WARNING__KEY (1006)
#define DLOGS_MESSAGE_INFO__KEY (1007)

#define DLOGS_MESSAGE_BADPOINTER__VALUE ("Bad pointer to")
#define DLOGS_MESSAGE_BADVALUE__VALUE ("Bad value")
#define DLOGS_MESSAGE_FUNCFAIL__VALUE ("Function fail")
#define DLOGS_MESSAGE_BADALLOC__VALUE ("Can't alloc memory")
#define DLOGS_MESSAGE_ERROR__VALUE ("Error")
#define DLOGS_MESSAGE_WARNING__VALUE ("Warning")
#define DLOGS_MESSAGE_INFO__VALUE ("Info")

#define DLOGS_DEFINE_DEFAULT_CONTEXT DLogs::DLogsContext log_context;

#define DLOGS_GLOBAL_CONTEXT (log_context)

#define DLOGS_INIT_DEFAULT_CONTEXT(STREAM_NAME) \
    log_context = DLogs::DLogsContext(STREAM_NAME); \
    log_context.header_set_all(false, true, false, false, true, true); \
    log_context.add_message(DLOGS_MESSAGE_BADPOINTER__VALUE, DLOGS_MESSAGE_BADPOINTER__KEY); \
    log_context.add_message(DLOGS_MESSAGE_BADVALUE__VALUE, DLOGS_MESSAGE_BADVALUE__KEY); \
    log_context.add_message(DLOGS_MESSAGE_FUNCFAIL__VALUE, DLOGS_MESSAGE_FUNCFAIL__KEY); \
    log_context.add_message(DLOGS_MESSAGE_BADALLOC__VALUE, DLOGS_MESSAGE_BADALLOC__KEY); \
    log_context.add_message(DLOGS_MESSAGE_ERROR__VALUE, DLOGS_MESSAGE_ERROR__KEY); \
    log_context.add_message(DLOGS_MESSAGE_INFO__VALUE, DLOGS_MESSAGE_INFO__KEY); \
    log_context.add_message(DLOGS_MESSAGE_WARNING__VALUE, DLOGS_MESSAGE_WARNING__KEY);

#define DLOGS_INIT_GLOBAL_CONTEXT(STREAM_NAME, Initializator) \
    DLogs::DLogsContextInitializator Initializator(STREAM_NAME, log_context);

/*
#define DL_BADPOINTER(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_BADPOINTER__KEY), __VA_ARGS__)
#define DL_BADVALUE(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_BADVALUE__KEY), __VA_ARGS__)
#define DL_FUNCFAIL(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_FUNCFAIL__KEY), __VA_ARGS__)
#define DL_BADALLOC(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_BADALLOC__KEY), __VA_ARGS__)
#define DL_ERROR(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_ERROR__KEY), __VA_ARGS__)
#define DL_WARNING(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_WARNING__KEY), __VA_ARGS__)
#define DL_INFO(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, log_context.header_set_message(DLOGS_MESSAGE_INFO__KEY), __VA_ARGS__)
*/
#define DL_BADPOINTER(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_BADPOINTER__KEY, &log_context, __VA_ARGS__)
#define DL_BADVALUE(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_BADVALUE__KEY, &log_context, __VA_ARGS__)
#define DL_FUNCFAIL(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_FUNCFAIL__KEY, &log_context, __VA_ARGS__)
#define DL_BADALLOC(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_BADALLOC__KEY,&log_context, __VA_ARGS__)
#define DL_ERROR(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_ERROR__KEY, &log_context, __VA_ARGS__)
#define DL_WARNING(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_WARNING__KEY, &log_context, __VA_ARGS__)
#define DL_INFO(level, ...) DLogs::dlogs_base(level, D_FUNC_NAME, DLOGS_MESSAGE_INFO__KEY, &log_context, __VA_ARGS__)


namespace DLogs {
struct DLogsContextInitializator {
    DLogsContextInitializator(const char *streamName, DLogsContext &log_context) {
        DLOGS_INIT_DEFAULT_CONTEXT(streamName);
        log_context.set_lvl_cmp_callback(DLogs::default_lvl_cmp__less_oe);
        log_context.set_log_level(1);
    }
};
}
#endif // DLOGS_H
