#include "test_string.h"
#include "test_map.h"
#include "test_vector.h"
#include "test_list.h"
#include "test_set.h"
#include "test_pair.h"
#include "test_stack.h"
#include "test_queue.h"
#include "test_class.h"
#include "test_class_child.h"
#include "test_func.h"
#include "test_struct.h"
#include "test_algorithm.h"
#include "test_ext_hash_map.h"
#include "test_ext_hash_set.h"
#include "test_ext_hash_slist.h"
#include "test_iterator.h"
#include "test_deque.h"
#include "parseURL.h"
#include "Http.h"
#include "MD5.h"
#include "DB.h"
#include "mark_oss.h"
#include "ConfigParser.h"
#include "parseHTML.h"
#include "test_regex.h"


namespace __gnu_cxx
{
    template<> struct hash<const std::string>
    {
        size_t operator()(const std::string& s) const{
            return hash<const char*>()( s.c_str() );
        }
    };
    template<> struct hash<std::string>{
        size_t operator()(const std::string& s) const{
            return hash<const char*>()( s.c_str() );
        }
    };
}
