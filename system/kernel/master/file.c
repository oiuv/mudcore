string creator_file(string str) {
    // debug_message("[CORE_MASTER_OB]->creator_file():" + str);
    return (string)call_other(SIMUL_EFUN_OB, "creator_file", str);
}

string domain_file(string str) {
    // debug_message("[CORE_MASTER_OB]->domain_file():" + str);
    return (string)call_other(SIMUL_EFUN_OB, "domain_file", str);
}

string author_file(string str) {
    // debug_message("[CORE_MASTER_OB]->author_file():" + str);
    return (string)call_other(SIMUL_EFUN_OB, "author_file", str);
}

string privs_file(string file) {
    return file;
}
