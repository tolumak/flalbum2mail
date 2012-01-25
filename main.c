#include <stdio.h>
#include <flickcurl.h>
#include <getopt.h>


flickcurl_photoset * find_set(flickcurl *fc, char *name)
{
	int i;
	flickcurl_photoset ** sets;
	flickcurl_photoset *set, *ret = NULL;

	sets = flickcurl_photosets_getList(fc, NULL);
	if (!sets)
		return NULL;

	for (set = sets[0], i = 0 ; set ; set = sets[++i]) {
		if (!strcmp(name, set->title)) {
			ret = flickcurl_photosets_getInfo(fc, set->id);
			goto out;
		}
	}

out:
	flickcurl_free_photosets(sets);
	return ret;
}


int main(int argc, char ** argv)
{
	int c;
	char * album_name = NULL;
	int auth_is_set = 0;
	int ret = 1;
	flickcurl *fc;

	flickcurl_init(); /* optional static initialising of resources */
	fc=flickcurl_new();

	/* Set configuration, or more likely read from a config file */
	flickcurl_set_api_key(fc, "...");
	flickcurl_set_shared_secret(fc, "...");


	static struct option long_options[] = {
		{"auth-token", required_argument, 0, 0},
		{"album-name", required_argument, 0, 0},
		{0, 0, 0, 0}
	};

	while(1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "",
				long_options, &option_index);
		if (c == -1)
			break;
		if (c == 0) {
			switch(option_index) {
			case 0:
				flickcurl_set_auth_token(fc, optarg);
				auth_is_set = 1;
				break;
			case 1:
				album_name = optarg;
				break;
			}
		}
	}

	if (!auth_is_set) {
		fprintf(stderr, "Authentification token required\n");
		goto out;
	}

	if (!album_name) {
		fprintf(stderr, "Album name required\n");
		goto out;
	}


	ret = 0;

out:
	flickcurl_free(fc);
	flickcurl_finish(); /* optional static free of resources */

	return ret;

}
