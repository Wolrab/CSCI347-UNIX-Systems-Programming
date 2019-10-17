#include <regex.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char errbuff[4096];
    FILE *f = fopen("/etc/passwd", "r");
    char *line = NULL;
    size_t len;
    regex_t reg;
    regmatch_t match[2];
    int ret;
    getline(&line, &len, f);
    printf("%s", line);
    //ret = regcomp(&reg, "([:print:]*):[:print:]*:0", 0);
    ret = regcomp(&reg, "^\\([[:print:]]*\\):[[:print:]]*:0:0", 0);
    if (ret) {
        regerror(ret, &reg, errbuff, 4096);
        printf("%s\n", errbuff);   
    }
    ret = regexec(&reg, line, 2, match, 0);
    if (ret == REG_NOMATCH) {
        printf("No match\n");
    }
    else {
        strncpy(errbuff, line + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
        errbuff[match[1].rm_eo + 1] = '\0';
        printf("%s\n", errbuff);
    }
    return 0;
}