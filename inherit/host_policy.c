// Reports implementation ownership, never whether an authorization policy is safe.
mapping inspect_host_policy(object masterObject) {
    mapping checks;
    string method, source, corePath, status;
    string *warnings;
    mixed err;

    if (!objectp(masterObject)) error("HOST_POLICY: expected a loaded master object.\n");
    checks = ([]);
    warnings = ({});
    corePath = CORE_MASTER_OB;
    if (corePath[0] != '/') corePath = "/" + corePath;
    foreach (method in ({ "valid_read", "valid_write", "valid_socket", "valid_seteuid",
        "valid_bind", "valid_database" })) {
        source = 0;
        err = catch(source = efun::function_exists(method, masterObject, 1));
        status = "unknown";
        if (!err && stringp(source) && source != "") {
            if (source[0] != '/') source = "/" + source;
            if (sizeof(source) > 4 && source[<4..] == ".lpc") source = source[0..<5];
            else if (sizeof(source) > 2 && source[<2..] == ".c") source = source[0..<3];
            status = source == corePath ? "framework-default" : "host-defined";
        }
        checks[method] = ([ "source": source, "status": status ]);
        if (status == "unknown") warnings += ({ method + ": implementation source unavailable." });
        if (status == "framework-default" &&
            member_array(method, ({ "valid_read", "valid_write", "valid_socket" })) != -1)
            warnings += ({ method + ": permissive framework default; review the host policy." });
    }
    return ([
        "checks": checks,
        "warnings": warnings,
        "notice": "Implementation origin only, not a security audit. Host-defined does not mean safe; shadows are not inspected."
    ]);
}
