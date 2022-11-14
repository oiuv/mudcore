// preload.c

string *epilog(int load_empty)
{
    string *preload_list = ({});
    debug_message("[" + ctime() + "]CORE_MASTER_OB->epilog()!");
#ifdef PRELOAD
    preload_list = read_lines(PRELOAD);
    foreach (string path in preload_list)
    {
        string file;

        if (path[ < 1] == '/' && file_size(path) == -2)
        {
            foreach (file in get_dir(path))
            {
                if (file[ < 2.. < 1] == ".c" && file_size(path + file) > 0)
                    preload_list += ({path + file});
            }
            preload_list -= ({path});
        }
    }
#endif
    return preload_list;
}

// preload an object
void preload(string file)
{
    string err;

    err = catch (load_object(file));
    if (err != 0)
    {
        write("[CORE_MASTER_OB]->preload():Error " + err + " when loading " + file + "\n");
    }
}
