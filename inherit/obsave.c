// obsave.c

int is_ob_saved() { return 1; }

int save()
{
    // debug_message("[OBSAVE]->save()!");
    CORE_DBASE_D->set_data(this_object()->save_dbase_data());
    return 1;
}

int restore()
{
    // debug_message("[OBSAVE]->restore()!");
    this_object()->receive_dbase_data(CORE_DBASE_D->query_data());
    return 1;
}
