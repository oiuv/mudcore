// preload.c

string *epilog(int load_empty)
{
    string *items = ({});
    debug_message("[" + ctime() + "]MASTER_OB->epilog()!");
#ifdef PRELOAD
    items = update_file(PRELOAD);
#endif
    return items;
}

// preload an object
void preload(string file)
{
    string err;
    // debug_message("[MASTER_OB]->preload()!");
    if (file_size(file + ".c") == -1)
        return;

    err = catch(call_other(file, "??"));
    if (err != 0) {
        write("[MASTER_OB]->preload():Error " + err + " when loading " + file + "\n");
    }
}
