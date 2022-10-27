/* Import a binary file */
#define IMPORT_BIN(sect, file, sym) asm (\
    ".section " #sect "\n"                  /* Change section */\
    ".balign 4\n"                           /* Word alignment */\
    ".global " #sym "\n"                    /* Export the object address */\
    #sym ":\n"                              /* Define the object label */\
    ".incbin \"" file "\"\n"                /* Import the file */\
    ".global _sizeof_" #sym "\n"            /* Export the object size */\
	".set _sizeof_"#sym", . - " #sym "\n"   /* Define the object size */\
    ".balign 4\n"                           /* Word alignment */\
    ".section \".text\"\n")                 /* Restore section */

/* Import a part of binary file */
#define IMPORT_BIN_PART(sect, file, ofs, siz, sym) asm (\
    ".section " #sect "\n"\
    ".balign 4\n"\
    ".global " #sym "\n"\
    #sym ":\n"\
    ".incbin \"" file "\"," #ofs "," #siz "\n"\
    ".global _sizeof_" #sym "\n"\
    ".set _sizeof_" #sym ", . - " #sym "\n"\
    ".balign 4\n"\
    ".section \".text\"\n")

