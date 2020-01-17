#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

//#define for_each(item, list) for(T * item = list->head; item != NULL; item = item->next)

#define SYSTEM_COMMAND_MAX_SIZE 500
#define SYSTEM_STDOUT_MAX_SIZE 5000

//const char* allsets[] = { "china1", "china2", NULL };
//const char* chinasets[] = { "china1", "china2", NULL };

//const char* customset[][] = NULL;

//const char* config_file = "example.conf";

//const char** get_current_sets_all( const char* ); // need to free
const char** get_current_sets_spec( const char*, const char** ); // may need to free
void add_ip( const char*, const char* );
//void add_ip_timeout( const char*, const char*, const char* );  //timeout in string
void add_ip_timeout_s( const char*, const char*, int );  // timeout in second
void delete_ip_sets( const char*, const char** );
void delete_ip_set( const char*, const char* );
void print_route( const char** );
void print_route_json( const char* ipaddr, int bool_reset, int group_amount, const int* grouped_setsn, const int* routeget, const char** groupname, const char** current_sets, const char*** sets );
const char** getgroupname( const char* ); // need to free
const char*** parse_conf( const char* ); // need to free
char* file2str( const char* ); // need to free
const char** getallsets( const char*** ); // need to free
void free_triple( void *** );
void free_double( void ** );
int* setninsets( const char*** ); // need to free
int setninall( const char** );
int groupn( const char*** );
//int* setsn( const char*** );

int main(int argc, const char* argv[]){
	const char* ip = argv[1];
	const char* reset = argv[2];
	const char* config_file = argv[3];
	const int route_modify_start = 4;
//	int chinaroute = atoi(argv[3]);
//	int timeout = atoi(argv[4]);
//	int mark = 0x0, mask = 0x0;
#ifdef DEBUG
	fprintf(stderr, "in function main()\n");
//	fprintf(stderr, "%s %s %d %d\n", ip, reset, chinaroute, timeout); //
#endif
	const char** groupname = getgroupname(config_file);
	const char*** sets = parse_conf(config_file);
	const char** allsets = getallsets(sets);
	int group_amount = groupn(sets);
	int* grouped_setsn = setninsets(sets);
	int* routeget = malloc(group_amount * sizeof(int));
	for (int i=0; sets[i] != NULL; i++){
		routeget[i] = atoi(argv[(route_modify_start)+i]);
	}
	int timeout = atoi(argv[(route_modify_start)+group_amount]);
	// printf("Your IP is %s\n", ip);

	char** current_sets = NULL;
	if( strcmp( reset, "reset" ) == 0 ){
		// printf( "Reseting all your routes to default\n" );
		delete_ip_sets(ip, allsets);
		current_sets = get_current_sets_spec(ip, allsets);
		// print_route(current_sets);
		// print_route_json(ip, current_sets);
		print_route_json( ip, 1, group_amount, grouped_setsn, routeget, groupname, current_sets, sets );
		free(current_sets);
		free(allsets);
		free_double(groupname);
		free_triple(sets);
		free(routeget);
		free(grouped_setsn);
		return 0;
	}
/*
	if ( chinaroute == 0 ){
		printf( "Not modifying your china routes\n" );
	} else if ( chinaroute == 9 ){
		printf( "Reseting your china route to default\n" );
		delete_ip_sets(ip, chinasets);
	} else if ( chinaroute == 1 ){
		printf( "Changing your china route to route 1\n" );
		delete_ip_sets(ip, chinasets);
		add_ip(ip, "china1");
	} else if ( chinaroute == 2 ){
		printf( "Changing your china route to route 2\n" );
		delete_ip_sets(ip, chinasets);
		add_ip(ip, "china2");
	}
*/
	for (int i=0; sets[i] != NULL; i++){
		switch(routeget[i]){
			case 0:
				// printf("Not modifying your %s routes\n", groupname[i]);
				break;
			case 9:
				// printf("Reseting your %s route to default\n", groupname[i]);
				delete_ip_sets(ip, sets[i]);
				break;
			default:
				if(routeget[i] <= grouped_setsn[i]){
					// printf("Changing your %s route to route %d\n", groupname[i], routeget[i]);
					delete_ip_sets(ip, sets[i]);
					if(timeout == 0){
						add_ip(ip, sets[i][routeget[i] - 1]);
					}else{
						add_ip_timeout_s(ip, sets[i][routeget[i] - 1], timeout);
					}
				}
		}
	}
	current_sets = get_current_sets_spec(ip, allsets);
#ifdef DEBUG	
	if (current_sets[0] == NULL){
 		fprintf (stderr, "current_sets[0] is NULL\n");
	}else{
		fprintf (stderr, "current_sets[0] is not NULL\n");
	} 
#endif
	// print_route(current_sets);
	// print_route_json(ip, current_sets);
	print_route_json( ip, 0, group_amount, grouped_setsn, routeget, groupname, current_sets, sets );
	free(current_sets);
//	free_double(allsets);
	free(allsets);
	free_double(groupname);
	free_triple(sets);
	free(routeget);
	free(grouped_setsn);
#ifdef DEBUG
	fprintf(stderr, "Leaving function main()\n");
#endif
	return 0;
}

/*
const char** get_current_sets_all( const char* ip ){
	return get_current_sets_spec(ip, allsets);
}
*/

const char** get_current_sets_spec( const char* ip, const char** sets_to_get ){
#ifdef DEBUG
	fprintf(stderr, "In function get_current_sets_spec()\n");
#endif
	const char* current_sets[10] = { NULL };
	int current_point = 0;
	for (int i=0; sets_to_get[i] != NULL; i++ ){
		const char* current_set = sets_to_get[i];
		char command[SYSTEM_COMMAND_MAX_SIZE] = "", result[SYSTEM_STDOUT_MAX_SIZE] = "";
		const cJSON *table = NULL;
		const cJSON *set = NULL;
		const cJSON *name = NULL;
		const cJSON *elements = NULL;
		const cJSON *element = NULL;
		const cJSON *subelement = NULL;
		const cJSON *val = NULL;
		const cJSON *timeout = NULL;
		const cJSON *expires = NULL;
		sprintf(command, "nft --json list set ip mangle %s", current_set);
#ifdef DEBUG
		fprintf(stderr, command); fprintf(stderr, "\n"); //
#endif
		FILE* pipe = popen(command, "r");
		fgets( result, (SYSTEM_STDOUT_MAX_SIZE - 1), pipe );
		pclose(pipe);
#ifdef DEBUG
		fprintf(stderr, result);fprintf(stderr, "\n"); //
#endif
		if ( result[0] != '{' ){
			continue;
		}
		cJSON *rules = cJSON_Parse(result);  //malloc
		table = cJSON_GetObjectItemCaseSensitive(rules, "nftables");
		set = cJSON_GetObjectItemCaseSensitive(table -> child, "set");
		name = cJSON_GetObjectItemCaseSensitive(set, "name");
		elements = cJSON_GetObjectItemCaseSensitive(set, "elem");
		if ( cJSON_IsArray(elements) ){
			cJSON_ArrayForEach(element,elements){
				const char* current_ip = NULL;
				if ( cJSON_IsObject(element) ){
					subelement = cJSON_GetObjectItemCaseSensitive(element, "elem");
					val = cJSON_GetObjectItemCaseSensitive(subelement, "val");
					timeout = cJSON_GetObjectItemCaseSensitive(subelement, "timeout");
					expires = cJSON_GetObjectItemCaseSensitive(subelement, "expires");
					if (cJSON_IsString(val) && (val->valuestring != NULL)){
						current_ip = cJSON_GetStringValue(val);
#ifdef DEBUG
						fprintf(stderr, current_ip);fprintf(stderr, "\n"); //
#endif
					}
				}else{
					if (cJSON_IsString(element) && (element->valuestring != NULL)){
						current_ip = cJSON_GetStringValue(element);
#ifdef DEBUG
						fprintf(stderr, current_ip);fprintf(stderr, "\n"); //
#endif
					}
				}
				if (strcmp(current_ip, ip) == 0){
					current_sets[current_point] = current_set;
					current_point++;
					break;
				}
			}
		}
		cJSON_Delete(rules);  //free
	}
#ifdef DEBUG
	fprintf(stderr, "current_point = %d\n", current_point); //
#endif
	const char** sets_found = malloc((current_point+1)*sizeof(const char*));
	for (int i = 0; i <= current_point; i++){
#ifdef DEBUG
		fprintf(stderr, "i = %d\n",i); //
#endif
		sets_found[i] = current_sets[i];
#ifdef DEBUG
		if (sets_found[i] != NULL){
			fprintf(stderr, "sets_found[%d] is %s\n", i, sets_found[i]);
		}else{
			fprintf(stderr, "sets_found[%d] is NULL\n", i);
		}
#endif
	}
#ifdef DEBUG
	fprintf(stderr, "returning sets\n"); //
	fprintf(stderr, "Leaving function get_current_sets_spec()\n");
#endif
	return sets_found;
}

void print_route(const char** current_sets){
#ifdef DEBUG
	fprintf(stderr, "In function print_route()\n");
	if (*current_sets == NULL){
		fprintf(stderr, "*current_sets is NULL\n");
	}else{
		fprintf(stderr, "*current_sets is %s\n", *current_sets);
	}
#endif
	printf("Your route is ");
#ifdef DEBUG
	printf("\n"); //
#endif
	if (current_sets[0] == NULL){
		printf("default ");
	}
	for (int i=0; current_sets[i] != NULL; i++){
//	for (const char* current_set_ptr = current_sets; *current_set_ptr != NULL; current_set_ptr++){
//		const char* current_set = *current_set_ptr;
		const char *current_set = current_sets[i];
		printf("%s ",current_set);
#ifdef DEBUG
		printf("\n"); //
#endif
	}
	printf("\n");
#ifdef DEBUG
	fprintf(stderr, "Leaving function print_route()\n");
#endif
}

void print_route_json( const char* ipaddr, int bool_reset, int group_amount, const int* grouped_setsn, const int* routeget, const char** groupname, const char** current_sets, const char*** sets ){
	cJSON *currentroutes = NULL;
	cJSON *ip = NULL;
	cJSON *reset = NULL;
	cJSON *modify = NULL;
	cJSON *moded = NULL;
	cJSON *set = NULL;
	cJSON *response = cJSON_CreateObject();

	ip = cJSON_CreateString(ipaddr);
	cJSON_AddItemToObject(response, "IP", ip);

	reset = cJSON_CreateBool(bool_reset);
	cJSON_AddItemToObject(response, "reset", reset);

	if (!bool_reset){
	modify = cJSON_CreateObject();
		for (int i = 0; i <= group_amount - 1; i++){
			switch (routeget[i]){
				case 0 :
					cJSON_AddItemToObject(modify, groupname[i], cJSON_CreateNull());
					// cJSON_AddItemToObject(modify, groupname[i], NULL);
					break;
				case 9:
					cJSON_AddItemToObject(modify, groupname[i], cJSON_CreateString("reset"));
					break;
				default:
					if(routeget[i] <= grouped_setsn[i]){
						cJSON_AddItemToObject(modify, groupname[i], cJSON_CreateString(sets[i][routeget[i] - 1]));
					}else{
						// cJSON_AddItemToObject(modify, groupname[i], NULL);
					}
			}
		}
		cJSON_AddItemToObject(response, "modify", modify);
	}

	currentroutes = cJSON_CreateArray();
	cJSON_AddItemToObject(response, "currentroutes", currentroutes);
	for (int i=0; current_sets[i] != NULL; i++){
		set = cJSON_CreateString(current_sets[i]);
		cJSON_AddItemToArray(currentroutes, set);
	}
	char* json_to_print = cJSON_PrintUnformatted(response);
//	char* json_to_print = cJSON_Print(response);
	printf("%s\n", json_to_print);
	cJSON_Delete(response);
	free(json_to_print);
}
/*
 * Examples: (formatted)
 * {
 * 	"IP": "127.0.0.1",
 * 	"reset": false,
 * 	"modify": {
 * 		"china": "china1",
 * 		"europe": null,
 * 		"global": "reset"
 * 	},
 * 	"currentroutes": ["china1", "europe1"]
 * }
 * or
 * {
 * 	"IP": "127.0.0.1",
 * 	"reset": true,
 * 	"currentroutes": []
 * }
 */
void add_ip( const char* ip, const char* set_to_add ){
	char command[SYSTEM_COMMAND_MAX_SIZE] = "";
	sprintf(command, "nft add element ip mangle %s { %s }", set_to_add, ip);
#ifdef DEBUG
	fprintf(stderr, command); fprintf(stderr, "\n"); //
#endif
	system(command);
}

void add_ip_timeout_s( const char* ip, const char* set_to_add, int timeout){
	char command[SYSTEM_COMMAND_MAX_SIZE] = "";
	sprintf(command, "nft add element ip mangle %s { %s timeout %ds }", set_to_add, ip, timeout);
#ifdef DEBUG
	fprintf(stderr, command); fprintf(stderr, "\n");
#endif
	system(command);
}

void delete_ip_set( const char* ip, const char* set_to_delete ){
	char command[SYSTEM_COMMAND_MAX_SIZE] = "";
	sprintf(command, "nft delete element ip mangle %s { %s }", set_to_delete, ip);
#ifdef DEBUG
	fprintf(stderr, command); fprintf(stderr, "\n"); //
#endif
	system(command);
}

void delete_ip_sets( const char* ip, const char** sets_may_delete){
	const char** sets_to_delete = get_current_sets_spec(ip, sets_may_delete);
	for (int i=0; sets_to_delete[i] != NULL; i++){
//	for (const char** current_set_ptr = sets_to_delete; *current_set_ptr != NULL; current_set_ptr++){
//		const char* current_set = *current_set_ptr;
		const char* current_set = sets_to_delete[i];
		delete_ip_set(ip, current_set);
	}
	free(sets_to_delete);
}

const char** getgroupname( const char* conffile ){
	const char* confstr = file2str(conffile);
	const cJSON *conf = cJSON_Parse(confstr);
	free(confstr);
	const cJSON *setsgroup = cJSON_GetObjectItemCaseSensitive(conf, "sets");
	int setsingroup = 0;
	cJSON *sets = NULL;
	cJSON_ArrayForEach(sets, setsgroup){
		setsingroup++;
	}
	char** groupsname = NULL;
	groupsname = calloc((setsingroup + 1), sizeof(char*));
	int groupcount = 0;
	cJSON_ArrayForEach(sets, setsgroup){
		int namelength = strlen(sets -> string);
		groupsname[groupcount] = calloc(namelength + 1, sizeof(char));
		strcpy(groupsname[groupcount], sets -> string);
		groupcount++;
	}
	cJSON_Delete(conf);
	return groupsname;
}

const char*** parse_conf( const char* conffile ){
#ifdef DEBUG
	fprintf(stderr, "In function parse_conf()\n");
#endif
	const char* confstr = file2str(conffile);
	const cJSON *conf = cJSON_Parse(confstr);
	free(confstr);
	const cJSON *setsgroup = cJSON_GetObjectItemCaseSensitive(conf, "sets");
	int setsingroup = 0;
	cJSON *sets = NULL;
	cJSON_ArrayForEach(sets, setsgroup){
		setsingroup++;
	}
#ifdef DEBUG
	fprintf(stderr, "setsingroup=%d\n", setsingroup);
#endif
	char*** groupedsets = NULL;
	groupedsets = calloc((setsingroup + 1), sizeof(char**));
	int setsnumber = 0;
	cJSON_ArrayForEach(sets, setsgroup){
		const char* groupname = sets -> string; // groupname is temporarily only for debug
		int setinsets = 0;
		cJSON *set = NULL;
		cJSON_ArrayForEach(set, sets){
			setinsets++;
		}
#ifdef DEBUG
		fprintf(stderr, "(setsgroup[%d]=%s).setinsets=%d\n", setsnumber, groupname, setinsets);
#endif
		groupedsets[setsnumber] = calloc((setinsets + 1), sizeof(char*));
		int setnumber = 0;
		cJSON_ArrayForEach(set, sets){
#ifdef DEBUG
			fprintf(stderr, "set -> valuestring = %s\n", set -> valuestring);
#endif
			int setnamelength = strlen(set -> valuestring);
#ifdef DEBUG
			fprintf(stderr, "strlen(set -> valuestring) = %d\n", setnamelength);
#endif
			groupedsets[setsnumber][setnumber] = calloc((setnamelength + 1), sizeof(char));
			strcpy(groupedsets[setsnumber][setnumber], set -> valuestring);
#ifdef DEBUG
			fprintf(stderr, "groupedsets[%d][%d] is %s\n", setsnumber, setnumber, groupedsets[setsnumber][setnumber]);
#endif
			setnumber++;
		}
		setsnumber++;
	}
#ifdef DEBUG
	fprintf(stderr, "leaving function parse_conf()\n");
#endif
	cJSON_Delete(conf);
	return groupedsets;
}

char* file2str( const char* filename ){ 
#ifdef DEBUG
	fprintf(stderr, "In function file2str()\n");
#endif
	FILE *file = fopen(filename, "r");
	if (!file){
		fprintf(stderr, "Error opening file %s\n", filename);
//		fclose(file);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	int filelength = ftell(file);
	fseek(file, 0, SEEK_SET);
#ifdef DEBUG
	fprintf(stderr,"filelength = %d\n", filelength);
#endif
	char* filestr = calloc((filelength + 1), sizeof(char));
	char buffc; int p = 0;
	while((buffc = fgetc(file)) != EOF){
		filestr[p] = buffc;
		p++;
	}
//	fgets(filestr, filelength, file);
	fclose(file);
#ifdef DEBUG
	fprintf(stderr, "The read file in string is\n");
	fputs(filestr, stderr);
	fprintf(stderr, "\n");
	fprintf(stderr, "leaving file2str()\n");
#endif
	return filestr;
}

const char** getallsets( const char*** groupedsets ){
	char** allsets = NULL;
	int setamount = 0;
	for (int setsnumber = 0; groupedsets[setsnumber] != NULL; setsnumber++){
		for (int setnumber = 0; groupedsets[setsnumber][setnumber] != NULL; setnumber++){
			setamount++;
		}
	}
	allsets = calloc((setamount + 1), sizeof(char*));
	int setp = 0;
	for (int setsnumber = 0; groupedsets[setsnumber] != NULL; setsnumber++){
		for (int setnumber = 0; groupedsets[setsnumber][setnumber] != NULL; setnumber++){
			allsets[setp] = groupedsets[setsnumber][setnumber];
			setp++;
		}
	}
	return allsets;
}

void free_triple( void *** triple_array ){
#ifdef DEBUG
	fprintf(stderr, "In function free_triple()\n");
#endif
	for (int i = 0; triple_array[i] != NULL; i++){  // i from 0, triple_array+i aka &triple_array[i] is valid, i++
//		for (int j = 0; (triple_array[i] + j); j++){
//			free(triple_array[i][j]);
//		}
//		free(triple_array[i]);
		free_double(triple_array[i]);
	}
	free(triple_array);
#ifdef DEBUG
	fprintf(stderr, "leaving function free_triple()\n");
#endif
}

void free_double( void ** double_array ){
#ifdef DEBUG
	fprintf(stderr, "In function free_double()\n");
#endif
	for (int i = 0; double_array[i] != NULL; i++){
#ifdef DEBUG
		fprintf(stderr, "deleting double_array[%d]\n", i);
#endif
		free(double_array[i]);
	}
#ifdef DEBUG
	fprintf(stderr, "deleting double_array\n");
#endif
	free(double_array);
#ifdef DEBUG
	fprintf(stderr, "leaving function free_double()\n");
#endif
}

int* setninsets( const char*** sets ){
	int setscount = 0;
	for (int i=0; sets[i] != NULL; i++){
		setscount++;
	}
	int *setsamount = malloc(setscount * sizeof(int));
	for (int i=0; sets[i] != NULL; i++){
		setsamount[i] = setninall(sets[i]);
	}
	return setsamount;
}

int setninall( const char** setgroup ){
	int setcount = 0;
	for (int i=0; setgroup[i] != NULL; i++){
		setcount++;
	}
	return setcount;
}

int groupn( const char*** groups ){
	int groupcount = 0;
	for (int i=0; groups[i] != NULL; i++){
		groupcount++;
	}
	return groupcount;
}

