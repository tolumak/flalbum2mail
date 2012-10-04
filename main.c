#include <stdio.h>
#include <string.h>
#include <flickcurl.h>
#include <getopt.h>


struct photo_list_t {
	char * id;
	struct photo_list_t *next;
};

void free_photo_list(struct photo_list_t ** list)
{
	struct photo_list_t *elt = NULL, *tmp = NULL;
	elt = *list;
	while (elt) {
		tmp = elt;
		elt = elt->next;
		if (tmp->id)
			free(tmp->id);
		free(tmp);
	}
	*list = NULL;
}

int add_photo_list_elt(struct photo_list_t ** list, char *id)
{
	struct photo_list_t *elt = NULL;

	elt = malloc(sizeof(*elt));
	if (!elt)
		return -1;
	elt->id = strdup(id);
	elt->next = *list;
	*list = elt;
	return 0;
}

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

int build_flickr_photo_list(struct photo_list_t ** list,
			flickcurl *fc,
			flickcurl_photoset * set)
{
	int ret = -1;
	flickcurl_photo ** photos, **curr = NULL;
	flickcurl_photo * photo = NULL;

	*list = NULL;

	photos = flickcurl_photosets_getPhotos(fc, set->id, NULL, -1, -1, -1);

	if (!photos)
		goto out;

	curr = photos;
	if (curr)
		photo = *curr;
	while (photo) {
		if (add_photo_list_elt(list, photo->id) < 0)
			goto out;
		curr++;
		if (curr)
			photo = *curr;
	}

	ret = 0;

out:
	if (ret < 0) {
		free_photo_list(list);
	}

	if (photos)
		flickcurl_free_photos(photos);
	return ret;
}

int build_local_photo_list(struct photo_list_t ** list,
			    const char * album_name)

{
	char buf[256];
	FILE * f;
	int ret = -1;

	*list = NULL;

	f = fopen(album_name, "a+");
	if (!f)
		goto out;

	while(fgets(buf, sizeof(buf), f)) {
		buf[strlen(buf) - 1] = 0;
		if (add_photo_list_elt(list, buf) < 0)
			goto out;
	}

	ret = 0;
out:
	if (ret < 0) {
		free_photo_list(list);
	}

	fclose(f);
	return ret;
}

int store_photo_list(struct photo_list_t ** list,
	       const char * album_name)
{
	FILE * f;
	int ret = -1;
	struct photo_list_t *elt = NULL;

	f = fopen(album_name, "w");
	if (!f)
		goto out;

	elt = *list;
	while (elt) {
		fprintf(f, "%s\n", elt->id);
		elt = elt->next;
	}

	ret = 0;
out:
	fclose(f);
	return ret;

}

int compare_photo_lists(struct photo_list_t ** A, struct photo_list_t ** B,
		  struct photo_list_t ** added, struct photo_list_t ** removed)
{
	int bFound;
	struct photo_list_t *elt_A = NULL, *elt_B = NULL;

	elt_A = *A;
	while (elt_A) {
		bFound = 0;
		elt_B = *B;
		while (elt_B) {
			if (!strcmp(elt_A->id, elt_B->id)) {
				bFound = 1;
				break;
			}
			elt_B = elt_B->next;
		}

		if (!bFound) {
			add_photo_list_elt(added, elt_A->id);
		}

		elt_A = elt_A->next;
	}

	elt_B = *B;
	while (elt_B) {
		bFound = 0;
		elt_A = *A;
		while (elt_A) {
			if (!strcmp(elt_A->id, elt_B->id)) {
				bFound = 1;
				break;
			}
			elt_A = elt_A->next;
		}

		if (!bFound) {
			add_photo_list_elt(removed, elt_B->id);
		}

		elt_B = elt_B->next;
	}
	return 0;
}

int main(int argc, char ** argv)
{
	int c;
	char * album_name = NULL;
	int auth_is_set = 0;
	int ret = 1;
	flickcurl *fc;
	char * token;
	flickcurl_photoset * photoset= NULL;
	struct photo_list_t * flickr_list = NULL;
	struct photo_list_t * local_list = NULL;
	struct photo_list_t * added_list = NULL;
	struct photo_list_t * removed_list = NULL;
	struct photo_list_t * elt;

	flickcurl_init(); /* optional static initialising of resources */
	fc=flickcurl_new();

	/* Set configuration, or more likely read from a config file */
	flickcurl_set_api_key(fc, "eafe5948f68607d09447bda45c81ad56");
	flickcurl_set_shared_secret(fc, "e043b9fe5fa9ba51");


	static struct option long_options[] = {
		{"auth", required_argument, 0, 'u'},
		{"frob", required_argument, 0, 'f'},
		{"album-name", required_argument, 0, 'a'},
		{0, 0, 0, 0}
	};

	optind = 0;
	while(1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "", long_options,
				&option_index);
		if (c == -1)
			break;
		switch(c) {
		case 'u':
			if (optarg) {
				flickcurl_set_auth_token(fc, optarg);
				auth_is_set = 1;
			}
			break;
		case 'f':
			token = flickcurl_auth_getFullToken(fc, optarg);
			printf("Your authentification token is %s\n", token);
			if (token) {
				flickcurl_set_auth_token(fc, token);
				auth_is_set = 1;
			}
			break;
		case 'a':
			album_name = optarg;
			break;
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

	photoset = find_set(fc, album_name);
	if (!photoset) {
		fprintf(stderr, "Album not found\n");
		goto out;
	}

	printf("U http://www.flickr.com/photos/%s/sets/%s\n",
	       photoset->owner, photoset->id);

	if (build_flickr_photo_list(&flickr_list, fc, photoset) < 0) {
		fprintf(stderr, "Remote photo list not found\n");
		goto out;
	}

	if (build_local_photo_list(&local_list, album_name) < 0) {
		fprintf(stderr, "Local photo list not found\n");
		goto out;
	}

	if (compare_photo_lists(&flickr_list, &local_list,
				&added_list, &removed_list) < 0) {
		fprintf(stderr, "Unable to store remote list\n");
		goto out;
	}

	elt = added_list;
	while (elt) {
		flickcurl_photo * photo;
		photo = flickcurl_photos_getInfo(fc, elt->id);
		if (photo)
			printf("A %s %s\n", elt->id,
			       flickcurl_photo_as_source_uri(photo, 's')
				);
		elt = elt->next;
	}

	elt = removed_list;
	while (elt) {
		printf("D %s\n", elt->id);
		elt = elt->next;
	}

	if (store_photo_list(&flickr_list, album_name) < 0) {
		fprintf(stderr, "Unable to store remote list\n");
		goto out;
	}

	ret = 0;

out:
	if (photoset)
		flickcurl_free_photoset(photoset);

	if (flickr_list)
		free_photo_list(&flickr_list);

	if (local_list)
		free_photo_list(&local_list);

	if (added_list)
		free_photo_list(&added_list);

	if (removed_list)
		free_photo_list(&removed_list);


	flickcurl_free(fc);
	flickcurl_finish(); /* optional static free of resources */

	return ret;

}
