#include "config.h"

//code from lecture/notes
enum state { idle, want_in, in_cs };
shm int turn;
shm state flag[n]; // Flag corresponding to each process in shared memory
process ( const int i )
{
	int j; 
	do 
	{
		do 
		{
			flag[i] = want_in; // Raise my flag
			j = turn; // Set local variable
			while ( j != i )
				j = ( flag[j] != idle ) ? turn : ( j + 1 ) % n;
			// Declare intention to enter critical section
			flag[i] = in_cs;
			// Check that no one else is in critical section
			for ( j = 0; j < n; j++ )
				if ( ( j != i ) && ( flag[j] == in_cs ) )
					break;
		} while ( j < n ) || ( turn != i && flag[turn] != idle );

		//Assign turn to self and enter critical section
		turn = i;
		critical_selection();

		// Exit section
		j = (turn + 1) % n;
		while (flag[j] == idle)
			j = (j + 1) % n;
		
		// Assign turn to next waiting process; change own flag to idle
		turn = j;
		flag[i] = idle;

		remainder_section();
	} while (1);
}
