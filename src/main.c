/*****************************************************************************
 *   Ledger Plugin SDK
 *   (c) 2024 overcat
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/
#include "os.h"
#include "stellar/plugin.h"

// Functions implemented by the plugin
void handle_init_contract(stellar_plugin_init_contract_t *msg);
void handle_query_data_pair_count(stellar_plugin_query_data_pair_count_t *msg);
void handle_query_data_pair(stellar_plugin_query_data_pair_t *msg);

// Function to dispatch calls from the ethereum app.
static void dispatch_call(int message, void *parameters)
{
    if (parameters != NULL)
    {
        switch (message)
        {
        case STELLAR_PLUGIN_INIT_CONTRACT:
            handle_init_contract(parameters);
            break;
        case STELLAR_PLUGIN_QUERY_DATA_PAIR_COUNT:
            handle_query_data_pair_count(parameters);
            break;
        case STELLAR_PLUGIN_QUERY_DATA_PAIR:
            handle_query_data_pair(parameters);
            break;
        default:
            PRINTF("Unhandled message %d\n", message);
            break;
        }
    }
    else
    {
        PRINTF("Received null parameters\n");
    }
}

// Low-level main for plugins.
__attribute__((section(".boot"))) int main(int arg0)
{
    // Exit critical section
    __asm volatile("cpsie i");

    os_boot();

    BEGIN_TRY
    {
        TRY
        {
            // Check if plugin is called from the dashboard.
            if (!arg0)
            {
                // exit if the plugin is called from dashboard
                os_sched_exit(0);
            }
            else
            {
                // Not called from dashboard: called from the Stellar app!
                const unsigned int *args = (unsigned int *)arg0;

                // If `STELLAR_PLUGIN_CHECK_PRESENCE` is set, this means the caller is just trying to
                // know whether this app exists or not.
                if (args[0] != STELLAR_PLUGIN_CHECK_PRESENCE)
                {
                    dispatch_call(args[0], (void *)args[1]);
                }
            }

            // Call `os_lib_end`, go back to the Stellar app.
            os_lib_end();

            // Will not get reached.
            __builtin_unreachable();
        }
        CATCH_OTHER(e)
        {
            PRINTF("Exiting following exception: %d\n", e);
        }
        FINALLY
        {
            os_lib_end();
        }
    }
    END_TRY;
}
