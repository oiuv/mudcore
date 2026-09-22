// preload.c

string *epilog(int load_empty) {
    string *preload_list = ({});
    debug_message("[" + ctime() + "]CORE_MASTER_OB->epilog()!");
#ifdef PRELOAD
    preload_list = map(read_lines(PRELOAD), (: trim($1) :));
    foreach (string path in preload_list) {
        if (file_size(path) == -2) {
            preload_list += lpc_source_files(path);
            preload_list -= ({ path });
        }
    }
#endif
    return preload_list;
}

// preload an object
void preload(string file) {
    string err;

    err = catch(load_object(file));
    if (err != 0) {
        write("[CORE_MASTER_OB]->preload():Error " + err + " when loading " + file + "\n");
    }
}
