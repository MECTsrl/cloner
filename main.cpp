#include <QtGui/QApplication>
#include "cloner.h"
#include <QWSServer>
#include <getopt.h>

/* Long options */
static struct option long_options[] = {
	{"version", no_argument,        NULL, 'v'},
	{"qt", no_argument,        NULL, 'q'},
	{"qt", no_argument,        NULL, 'w'},
	{"qt", no_argument,        NULL, 's'},
	{NULL,      no_argument,        NULL,  0}
};

/*
 * Short options.
 * FIXME: KEEP THEIR LETTERS IN SYNC WITH THE RETURN VALUE
 * FROM THE LONG OPTIONS!
 */
static char short_options[] = "vqws";

static int application_options(int argc, char *argv[])
{
	int option_index = 0;
	int c = 0;

	if (argc <= 0)
		return 0;

	if (argv == NULL)
		return 1;

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
			case 'v':
                printf("%s version: rev.%d\n", argv[0], SVN_REV);
				exit(0);
				break;
			default:
				break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (application_options(argc, argv) != 0) {
		fprintf(stderr, "%s: command line option error.\n", __func__);

		return 1;
	}
    QApplication a(argc, argv);
    QWSServer::setCursorVisible(false);
    cloner w;
    w.showFullScreen();
    
    return a.exec();
}
