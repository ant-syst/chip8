
/*
	Keypad                   Keyboard
	+-+-+-+-+                +-+-+-+-+
	|1|2|3|C|                |1|2|3|4|
	+-+-+-+-+                +-+-+-+-+
	|4|5|6|D|                |Q|W|E|R|
	+-+-+-+-+       =>       +-+-+-+-+
	|7|8|9|E|                |A|S|D|F|
	+-+-+-+-+                +-+-+-+-+
	|A|0|B|F|                |Z|X|C|V|
	+-+-+-+-+                +-+-+-+-+
*/
// TODO doublons
static int nc_keys_mapping(int key)
{
	switch(key)
	{
		case '&':
			return KEY_1;
		case 'é':
			return KEY_2;
		case '"':
			return KEY_3;
		case '\'':
			return KEY_C;
		case 'q':
			return KEY_4;
		case 'w':
			return KEY_5;
		case 'e':
			return KEY_6;
		case 'r':
			return KEY_D;
		case 'a':
			return KEY_7;
		case 's':
			return KEY_8;
		case 'd':
			return KEY_9;
		case 'f':
			return KEY_E;
		case 'z':
			return KEY_A;
		case 'x':
			return KEY_0;
		case 'c':
			return KEY_B;
		case 'v':
			return KEY_F;
		default:
			fprintf(logfs, "IO_KEY_NOT_FOUND key %d\n", key);
			return IO_KEY_NOT_FOUND;
	}
}

int main(void)
{
	int i;
	static char pixels[N_LINES][N_COLS];
	static char keys[N_KEYS];
	struct io * io_ptr = NULL;

	logfs = fopen("log.txt", "w");
	if(!logfs)
		THROW2(error, 1, "fopen");

	io_ptr = nc_alloc(&nc_keys_mapping);
	if(!io_ptr)
		THROW2(error, 0, "nc_alloc");

    for(i=0; i<100; i++)
    {
		pixels[10][i] = 1;

		io_ptr->update(io_ptr, pixels);

		if(io_ptr->input_poll(io_ptr, keys) == CHIP8_QUIT)
			break;

		fprintf(logfs, "\n");
		for(int j=0; j<N_KEYS; j++)
		{
			if(keys[j])
				fprintf(logfs, "%2d => %d\n", j, (int)keys[j]);
		}

		fflush(logfs);

		sleep(2);
	}

    io_ptr->free(&io_ptr);

    fclose(logfs);

    return 0;

    error:

    return -1;
}
