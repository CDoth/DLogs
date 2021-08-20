#include "DLogs.h"

#include <daran.h>
#include <algorithm>

#define INNER_ERROR(description, error_code) \
    std::cout << " >>>>>>> DLogContext inner fault: error: [" << description << "] call: [" << D_FUNC_NAME << "] code: [" << error_code << "]" <<std::endl;

int DLogs2::default_console_parse(const char *line)
{
    printf("%s",line);
    return 0;
}
int DLogs2::default_file_parse(FILE *f, const char *line)
{
    fwrite(line, 1, strlen(line), f);
    return 0;
}
bool DLogs2::default_lvl_cmp__more(int base_level, int local_level)
{
    return local_level > base_level;
}
bool DLogs2::default_lvl_cmp__more_oe(int base_level, int local_level)
{
    return local_level >= base_level;
}
bool DLogs2::default_lvl_cmp__less(int base_level, int local_level)
{
    return local_level < base_level;
}
bool DLogs2::default_lvl_cmp__less_oe(int base_level, int local_level)
{
    return local_level <= base_level;
}
bool DLogs2::default_lvl_cmp__equal(int base_level, int local_level)
{
    return base_level == local_level;
}
void DLogs2::DLogsContext::init_all()
{
    is_console_available = true;
    is_file_available = false;
    is_special_available = false;

    c_print_callback = default_console_parse;
    f_print_callback = default_file_parse;
    s_print_callback = nullptr;
    lvl_cmp = default_lvl_cmp__less;

    header.cdate = false;
    header.fname = false;
    header.ltime = false;
    header.mnumb = false;
    header.rtime = false;
    header.sname = false;
    header.level = false;

    log_level = 0;

    message_counter = 0;

    stream_name = "Deafult";
    separator.append(10, '-');

    float_precision = 6;
    double_precision = 6;

    buffer_pos = 0;
    buffer = DArray<char>(DLOGS2_DEFAULT_BUFFER_SIZE);
    buffer_current_level = 0;
}

const char *DLogs2::DLogsContext::get_float_format()
{
    sprintf(format, "%%.%df ", float_precision);
    return format;
}
const char *DLogs2::DLogsContext::get_double_format()
{
    sprintf(format, "%%.%df ", double_precision);
    return format;
}
void DLogs2::DLogsContext::out(int level, const char *o)
{
    if(is_console_available && c_print_callback)
    {
        if(lvl_cmp(log_level, level))
            c_print_callback(o);
    }
    if(is_file_available && f_print_callback)
    {
        std::for_each(files.constBegin(), files.constEnd(), OpDLFileParser(f_print_callback, o, level));
    }
    if(is_special_available && s_print_callback)
    {
        s_print_callback(level, special_opaque, o);
    }
}
size_t DLogs2::DLogsContext::flush(int level)
{
//    std::cout << "===================== " << D_FUNC_NAME << " buffer_pos: " << buffer_pos << " buffer_size: " << buffer.size() << " buffer: " << buffer.begin() << std::endl;
    size_t r = 0;
    if(buffer_pos)
    {
        out(level, buffer.begin());
        zero_mem(buffer.begin(), buffer.size());
        r = buffer_pos;
        buffer_pos = 0;
    }
    return r;
}
size_t DLogs2::DLogsContext::flush(int level, const char *line)
{
    if(line)
    {
        out(level, line);
        return strlen(line);
    }
    return 0;
}
int DLogs2::DLogsContext::put_to_buffer(int level, const char *line)
{
    if(line)
    {
//        std::cout << D_FUNC_NAME << " line: " << line << std::endl;
        if(buffer_current_level != level)
        {
            flush(buffer_current_level);
            buffer_current_level = level;
        }
        char *buffer_begin = buffer.begin();
        int64_t l = strlen(line);
        size_t total_flushed = 0;
        while(l > 0 && total_flushed < DLOGS2_MAX_BYTES_PUT_TO_BUFFER)
        {
            size_t free_space = buffer.size() - buffer_pos;
            size_t r = snprintf(buffer_begin + buffer_pos, free_space, "%s", line + total_flushed);

//            std::cout << "---- put_to_buffer: " << r << std::endl;
            if(r < 0)
            {
                INNER_ERROR("Can't parse line to buffer", r);
                return r;
            }
            if(r >= free_space)
            {
                l -= free_space;
                total_flushed += free_space;
                flush(level);
            }
            else
            {
                l -= r;
                total_flushed += r;
                buffer_pos += r;
            }
        }
//        std::cout << D_FUNC_NAME << " total_flushed: " << total_flushed << std::endl;
        return total_flushed;
    }
//    return -1;
    return 0;
}



void DLogs2::DLogsContext::dlogs_separator_inner(int level)
{
    out(level, separator.c_str());
    print_new_line(level);
}

DLogs2::DLogsContext::DLogsContext()
{
    init_all();
//    std::cout << "Empty DLogsContext: " << this << std::endl;
}
DLogs2::DLogsContext::DLogsContext(const char *_stream_name)
{
    init_all();
    if(_stream_name)
        stream_name = _stream_name;
//    std::cout << "Spec DLogsContext: " << this << " stream name: " << _stream_name <<  std::endl;

}
DLogs2::DLogsContext::~DLogsContext()
{
//    std::cout << D_FUNC_NAME << " context: " << this << std::endl;
    FOR_VALUE(files.size(), i)
    {
        parse_file_state(files[i].target, 'r');
        fclose(files[i].target);
    }
}

//DLogs2::DLogsContext &DLogs2::DLogsContext::operator=(const DLogs2::DLogsContext &c)
//{

//}
void DLogs2::DLogsContext::parse_console(bool state)
{
    is_console_available = state;
}
void DLogs2::DLogsContext::parse_file(bool state)
{
    is_file_available = state;
}
void DLogs2::DLogsContext::parse_special(bool state)
{
    is_special_available = state;
}
void DLogs2::DLogsContext::set_lvl_cmp_callback(DLogs2::LOG_LEVEL_CMP_CALLBACK c)
{
    lvl_cmp = c;
}
void DLogs2::DLogsContext::parse_set_console_tool(DLogs2::PARSE_CONSOLE_TOOL tool)
{
    c_print_callback = tool;
}
void DLogs2::DLogsContext::parse_set_file_tool(DLogs2::PARSE_FILE_TOOL tool)
{
    f_print_callback = tool;
}
void DLogs2::DLogsContext::parse_set_special_tool(DLogs2::PARSE_SPECIAL_TOOL tool)
{
    s_print_callback = tool;
}

void DLogs2::DLogsContext::set_special_data(void *data)
{
    special_opaque = data;
}
void DLogs2::DLogsContext::file_add(const char *path, const char *key)
{
    FILE *target = fopen(path, "wb");
    if(target)
    {
        DLogsFile file;
        file.target = target;
        file.path = path;
        file.key = key;
        file.is_available = true;
        files.push_back(file);
        is_file_available = true;
        parse_file_state(target, 'a');
    }
}
#define FIND_FILE_BY_KEY(action) \
    std::string s_key = key; \
    FOR_VALUE(files.size(), i){ \
        if(files[i].key == s_key){ \
    DLogsFile &file = files[i]; \
            action; }}

void DLogs2::DLogsContext::file_lock(const char *key)
{
    if(key)
    {
        FIND_FILE_BY_KEY(file.is_available = false; parse_file_state(files[i].target, 'l'))
    }
}
void DLogs2::DLogsContext::file_unlock(const char *key)
{
    if(key)
    {
        FIND_FILE_BY_KEY(file.is_available = true; parse_file_state(files[i].target, 'u'))
    }
}
void DLogs2::DLogsContext::file_remove(const char *key)
{
    if(key)
    {
//        FIND_FILE_BY_KEY(files.remove(files[i]); parse_file_state(files[i].target, 'r'))
    }
}

void DLogs2::DLogsContext::file_add_level(const char *key, int level)
{
    if(key && level >= 0)
    {
        FIND_FILE_BY_KEY(if(file.levels.count(level) == 0) file.levels.push_back(level))
    }
}

void DLogs2::DLogsContext::file_remove_level(const char *key, int level)
{
    if(key && level >= 0)
    {
        FIND_FILE_BY_KEY(file.levels.remove(level))
    }
}
bool DLogs2::DLogsContext::file_is_available(const char *key)
{
    if(key)
    {
        FIND_FILE_BY_KEY(return file.is_available)
    }
    return false;
}
#undef RUN_BY_FILES

void DLogs2::DLogsContext::header_set(const DLogs2::header_state &hs)
{
    header = hs;
}
void DLogs2::DLogsContext::header_set_all(bool date, bool fname, bool ltime, bool rtime, bool mesnum, bool sname, bool level)
{
    header.cdate = date;
    header.fname = fname;
    header.ltime = ltime;
    header.mnumb = mesnum;
    header.rtime = rtime;
    header.sname = sname;
    header.level = level;
}
DLogs2::DLogsContext *DLogs2::DLogsContext::header_set_message(const char *message)
{
    if(message)
        header_message = message;
    return this;
}

DLogs2::DLogsContext *DLogs2::DLogsContext::header_set_message(int message_key)
{
    auto m = messages.find(message_key);
    if(m != messages.end())
        header_message = m->second;
    return this;
}

int DLogs2::DLogsContext::add_message(const char *m, int key)
{
    messages[key] = m;
    return key;
}
void DLogs2::DLogsContext::separator_set_size(int size)
{
    char s = separator.empty() ? '-' : separator.front();
    separator = std::string(size, s);
}
void DLogs2::DLogsContext::separator_set_symb(char symb)
{
    FOR_VALUE(separator.size(), i)
    {
        separator[i] = symb;
    }
}
void DLogs2::DLogsContext::precision_float(unsigned int p)
{
    if(p < 10)
        float_precision = p;
}
void DLogs2::DLogsContext::precision_double(unsigned int p)
{
    if(p < 10)
        double_precision = p;
}

char DLogs2::local_buffer[DLOGS2_LOCAL_BUFFER_SIZE];
#define LOAD_TO_BUFFER(...)\
    snprintf(local_buffer, DLOGS2_LOCAL_BUFFER_SIZE, __VA_ARGS__); \
    put_to_buffer(level, local_buffer);

void DLogs2::DLogsContext::add_header(int level, const char *caller_name)
{
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    int empty_header = 1;

    ++message_counter;
    if(header.cdate)
    {
        LOAD_TO_BUFFER("[D: %d.%d.%d]", now.tm_mday, now.tm_mon+1, now.tm_year+1900);
        empty_header = 0;
    }


    if(header.fname && caller_name)
    {
        LOAD_TO_BUFFER("[F: %s]", caller_name);
        empty_header = 0;
    }
    if(header.ltime)
    {
        LOAD_TO_BUFFER("[L: %d:%d:%d]", now.tm_hour, now.tm_min, now.tm_sec);
        empty_header = 0;
    }
    if(header.mnumb)
    {
        LOAD_TO_BUFFER("[N: %d]", message_counter);
        empty_header = 0;
    }
    if(header.rtime)
    {
        int ms = clock();
        int s = ms / 1000;  ms %= 1000;
        int m = s  / 60;    s  %= 60;
        int h = m  / 60;    m  %= 60;

        LOAD_TO_BUFFER("[R: %d:%d:%d:%d]", h, m, s, ms);
        empty_header = 0;
    }
    if(header.sname && !stream_name.empty())
    {
        LOAD_TO_BUFFER("[S: %s]", stream_name.c_str());
        empty_header = 0;
    }
    if(header.level)
    {
        LOAD_TO_BUFFER("[LV: %d]", level );
        empty_header = 0;
    }
    if(header_message.size())
    {
        if(empty_header == 0)
            add_space(level);
        LOAD_TO_BUFFER("%s:", header_message.c_str());
        empty_header = 0;
    }
    if(empty_header == 0)
        add_space(level);
}

int DLogs2::DLogsContext::parse_file_state(FILE *f, char state)
{
    if(f_print_callback)
    {
        int n = 0;
        switch (state)
        {
        case 'a':
            n = sprintf(local_buffer, "Start loging to this file");
            break;
        case 'r':
            n = sprintf(local_buffer, "Stop loging to this file");
            break;
        case 'l':
            n = sprintf(local_buffer, "This file locked for loging");
            break;
        case 'u':
            n = sprintf(local_buffer, "This file unlocked for loging");
            break;
        default:break;
        }

#define LOAD_TO_LOCAL(...) \
    n += snprintf(local_buffer + n, sizeof(local_buffer) - n, __VA_ARGS__)
        if(n)
        {
            time_t t = time(NULL);
            struct tm now = *localtime(&t);
            LOAD_TO_LOCAL( " <");
            LOAD_TO_LOCAL( "[D: %d.%d.%d]", now.tm_mday, now.tm_mon+1, now.tm_year+1900 );
            LOAD_TO_LOCAL( "[L: %d:%d:%d]", now.tm_hour, now.tm_min, now.tm_sec );
//            n += snprintf(local_buffer + n, sizeof(local_buffer) - n, "[D: %d.%d.%d]", now.tm_mday, now.tm_mon+1, now.tm_year+1900);
//            n += snprintf(local_buffer + n, sizeof(local_buffer) - n, "[L: %d:%d:%d]", now.tm_hour, now.tm_min, now.tm_sec);
            int ms = clock();
            int s = ms / 1000;  ms %= 1000;
            int m = s  / 60;    s  %= 60;
            int h = m  / 60;    m  %= 60;
            LOAD_TO_LOCAL( "[R: %d:%d:%d:%d]", h, m, s, ms );
            LOAD_TO_LOCAL( ">\n" );
//            n += snprintf(local_buffer + n, sizeof(local_buffer) - n, "[R: %d:%d:%d:%d]", h, m, s, ms);
//            snprintf(local_buffer + n, sizeof(local_buffer) - n, "\n");
        }
        f_print_callback(f, local_buffer);
        zero_mem(local_buffer, sizeof(local_buffer));
    }
#undef LOAD_TO_LOCAL


    return 0;
}
void DLogs2::DLogsContext::print_new_line(int level)
{
    out(level, "\n");
}
void DLogs2::DLogsContext::add_new_line(int level)
{
    if(buffer_pos < buffer.size())
        buffer[buffer_pos++] = '\n';
    else
        flush(level), add_new_line(level);
}
void DLogs2::DLogsContext::print_space(int level)
{
    out(level, " ");
}
void DLogs2::DLogsContext::add_space(int level)
{
    if(buffer_pos < buffer.size())
    {
        buffer[buffer_pos++] = ' ';
    }
    else
        flush(level), add_space(level);
}
size_t DLogs2::DLogsContext::set_buffer_size(int s)
{
    if(buffer.size() != s && s >= DLOGS2_MIN_BUFFER_SIZE)
    {
        flush(0);
        size_t old_size = buffer.size();
        buffer = DArray<char>(s);
        return old_size;
    }
    return buffer.size();
}
size_t DLogs2::DLogsContext::expand_buffer(int s)
{
    if(is_expandable_buffer)
    {
        if( s > buffer.size() && s <= buffer_max_expanded_size)
        {
            return set_buffer_size(s);
        }
    }
    return buffer.size();
}
void DLogs2::DLogsContext::set_log_level(int l)
{
    log_level = l;
}



DLogs2::DLogsMaster::DLogsMaster(DLogsContext *c, bool auto_nl) : context(c), auto_n(auto_nl), level(0)
{
}
DLogs2::DLogsMaster::DLogsMaster(DLogs2::DLogsContext *c, bool temporary, const char *caller_name) : context(c), auto_n(temporary), level(0)
{
    if(temporary)
        context->add_header(level, caller_name);
}
DLogs2::DLogsMaster::~DLogsMaster()
{
    if(auto_n)
        context->add_new_line(level);
    context->flush(level);
}

int DLogs2::DLogsMaster::setLevel(int l)
{
    int _l = level;
    level = l;
    return _l;
}

DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator()(int l)
{
    level = l;
    return *this;
}

DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator()(int l, const char *caller_name)
{
    level = l;
    context->add_header(level, caller_name);
    return *this;
}

DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(FILE *p)
{
    context->put_to_buffer(level, "[FILE: %p]", p);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(int v)
{
    context->put_to_buffer(level, "%d", v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(const char *v)
{
    context->put_to_buffer(level, v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(float v)
{
    context->put_to_buffer(level, context->get_float_format(), v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(double v)
{
    context->put_to_buffer(level, context->get_double_format(), v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(bool v)
{
    if(v) context->put_to_buffer(level, "true");
    else context->put_to_buffer(level, "false");
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(long v)
{
    context->put_to_buffer(level, "%ld", v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(char v)
{
    context->put_to_buffer(level, "%c", v);
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(std::nullptr_t)
{
    context->put_to_buffer(level, "nullptr");
    context->add_space(level);
    return *this;
}
DLogs2::DLogsMaster &DLogs2::DLogsMaster::operator <<(DLogs2::DLogsMaster &)
{
    context->flush(level);
    context->add_new_line(level);
    return *this;
}



void DLogs2::dlogs_separator(int level, DLogs2::DLogsContext *context)
{
    context->dlogs_separator_inner(level);
}





