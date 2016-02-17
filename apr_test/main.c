#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "apr_pools.h"
#include "apr_general.h"
#include "apr_strings.h"

#include "src/test_str.h"
#include "src/test_table.h"
#include "src/test_array.h"
#include "src/test_hash.h"
#include "src/test_ring.h"
#include "src/test_time.h"


int main(int argc, char* argv){

    test_str();

    return 0;
}
