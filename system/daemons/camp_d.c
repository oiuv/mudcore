#include <ansi.h>

#ifndef CAMP_DIR
#define CAMP_DIR CORE_DIR "/world/camp/"
#endif

void create()
{
    seteuid(getuid());
}

string getCampFile(string camp)
{
    string camp_file;
    camp_file = CAMP_DIR + camp + ".c";
    if (file_size(camp_file) == -1 || file_size(camp_file) == -2)
        return 0;
    if (camp_file->isCamp() == 0)
        return 0;
    return camp_file;
}

string getCampName(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return 0;
    return camp_file->getName();
}

string *getFriendly(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return ({});

    return camp_file->getFriendly();
}

string *getAdversely(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return ({});

    return camp_file->getAdversely();
}

int getHate(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return 0;
    return camp_file->getHat();
}

string getHateMsg(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return "";

    return camp_file->getHateMsg();
}

int getAdvHate(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return 0;

    return camp_file->getAdvHate();
}

string getAdvHateMsg(string camp)
{
    string camp_file;
    camp_file = getCampFile(camp);
    if (!stringp(camp_file))
        return "";

    return camp_file->getAdvHateMsg();
}
