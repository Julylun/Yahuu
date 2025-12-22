#include "services/peachdb/peachdb.h"

int main()
{
    //Initialize PeachDB
    Peach_initPeachDb();

    Peach_collection_create("User", "id^name^email");
    Peach_write_record("User", "1^John Doe^john.doe@example.com");

}
