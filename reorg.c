#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "utils.h"

Person *person_map;
unsigned int *knows_map;
unsigned long person_length, knows_length;

typedef struct {
    unsigned short location;
}Cache;

Cache *cache;

FILE *knows_map_file;
FILE *person_map_file;

int main(int argc, char *argv[]) {

	char* person_output_file = makepath(argv[1], "person_new", "bin");
	char* knows_output_file = makepath(argv[1], "knows_new", "bin");

	person_map   = (Person *)         mmapr(makepath(argv[1], "person",   "bin"), &person_length);
	knows_map    = (unsigned int *)   mmapr(makepath(argv[1], "knows",    "bin"), &knows_length);

	knows_map_file = fopen(knows_output_file, "wb");
	person_map_file = fopen( person_output_file, "wb");

	unsigned int person_offset;
	unsigned long other_person_offset, knows_offset;

	unsigned long knows_first_counter = 0;

	Person *person, *knows;
	cache = malloc(sizeof(Cache) * person_length / sizeof(Person));

	for(person_offset = 0; person_offset < person_length/sizeof(Person); person_offset++) {
		person = &person_map[person_offset];
		cache[person_offset].location = person->location;
	}

	for (person_offset = 0; person_offset < person_length/sizeof(Person); person_offset++) {

		New_person *new_person = malloc(sizeof(New_person));
		person = &person_map[person_offset];

		unsigned short knows_n_counter = 0;

		//for debugging
		if (person_offset > 0 && person_offset % REPORTING_N == 0) {
			printf("%.2f%%\n", 100 * (person_offset * 1.0 / (person_length / sizeof (Person))));
        }

		for (knows_offset = person->knows_first;
			knows_offset < person->knows_first + person->knows_n;
			knows_offset++) {

			other_person_offset = knows_map[knows_offset];
			knows = &person_map[other_person_offset];

			if (cache[person_offset].location != cache[other_person_offset].location){
				continue;
			}

			fwrite(&knows_map[knows_offset], sizeof(unsigned int), 1, knows_map_file);
			knows_n_counter++;
		}

		new_person->person_id = person->person_id;
		new_person->knows_first = knows_first_counter;
		new_person->interests_first = person->interests_first;
		new_person->birthday = person->birthday;
		new_person->knows_n = knows_n_counter;
		new_person->interest_n = person->interest_n;

		knows_first_counter += knows_n_counter;
		fwrite(new_person, sizeof(New_person), 1, person_map_file);
	}

	fclose(knows_map_file);
	fclose(person_map_file);
	return 0;
}