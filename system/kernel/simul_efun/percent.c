mixed percent(mixed num, mixed den) {
    if ((!intp(num) && !floatp(num)) || (!intp(den) && !floatp(den)))
        error("percent: expected numeric arguments.\n");
    if (floatp(num) || floatp(den))
        return num * 100.0 / den;
    else
        return num * 100 / den;
}

mixed percent_of(mixed percent, mixed base) {
    if ((!intp(percent) && !floatp(percent)) || (!intp(base) && !floatp(base)))
        error("percent_of: expected numeric arguments.\n");
    if (floatp(percent) || floatp(base))
        return percent * base / 100.0;
    else
        return percent * base / 100;
}
