#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <string.h>
#include <dos.h>

#define QUIET //you'll need to provide the DOS4GW version provided with Doom 0.4 for this to work

/* Add environment strings to be searched here */
char *paths_to_check[] = {"DOS4GPATH", "PATH"};
unsigned vector_number;
void __interrupt __far *old_vector;

void __far *shared_memory_vector;
char shared_buffer[753];

char *dos4g_path()
{
	static char fullpath[80];
	int i;
	for(i = 0; i < sizeof(paths_to_check) / sizeof(paths_to_check[0]); i++) 
	{
		_searchenv( "dos4gw.exe", paths_to_check[i], fullpath );
		if(fullpath[0])
			return(&fullpath);
	}
	for(i = 0; i < sizeof(paths_to_check) / sizeof(paths_to_check[0]); i++) 
	{
		_searchenv("dos4g.exe", paths_to_check[i], fullpath);
		if(fullpath[0])
			return(&fullpath);
	}
	return "dos4gw.exe";
}

void free_shared_memory()
{
	if (vector_number != 0)
	{
		printf("STUB: freeing vector 0x%x\n", vector_number);
		_dos_setvect(vector_number, old_vector);
	}
}

void find_shared_memory()
{
	unsigned vector;
	unsigned char __far *oldvect;
	
	for (vector = 0x60; vector < 0x68; vector++)
	{
		shared_memory_vector = (void __far*)_dos_getvect(vector);
		if (*(long *)shared_memory_vector == 1146048333)
		{
			printf("STUB: using shared memory from vector 0x%x\n", vector);
			old_vector = NULL;
			return;
		}
	}
	
	_fmemset((void far*)&shared_buffer, 0, sizeof(shared_buffer));
	*(long far*)&shared_buffer = 1146048333;
	shared_memory_vector = (void __far*)&shared_buffer;
	
	vector = 0x60;
	for (;;)
	{
		if (vector >= 0x68)
		{
			printf("STUB: Couldn't find a free vector\n");
			free_shared_memory();
			exit(1);
		}
		
		oldvect = (char __far *)_dos_getvect(vector);
		if (oldvect == NULL || *oldvect == 0xcf)
			break;
		vector++;
	}
	
	old_vector = oldvect;
	vector_number = vector;
	_dos_setvect(vector_number, (void __interrupt __far*)shared_memory_vector);
	
	printf("STUB: hooking vector 0x%x\n", vector_number);
}

void empty_function()
{
}

main(int argc, char *argv[])
{
	int res;
	int i;
	char *av[20];
	int arg_num = 2;
	printf("STUB: begin execution\n");
	av[0] = dos4g_path(); /* Locate the DOS/4G loader */
	av[1] = argv[0]; /* name of executable to run */
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "STUBPARM"))
			empty_function();
		else
			av[arg_num++] = argv[i];
	}
	av[arg_num] = NULL; /* end of list */
	
	find_shared_memory();
	#ifdef QUIET
	putenv("DOS4G=QUIET"); /* disables DOS/4G Copyright banner */
	#endif
	printf("STUB: Launching dos extender\n");
	res = spawnvp(0, av[0], av);
	if (res == -1)
	{
		printf("STUB: Couldn't spawn dos extender: %s\n", strerror(errno));
	}
	else if (res > 0)
	{
		printf("STUB: Doom crashed!\n");
	}
	
	free_shared_memory();
	exit(0);
}
