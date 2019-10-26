CC=gcc
CFLAGS=$$(pkg-config allegro-5 allegro_font-5 allegro_image-5 allegro_primitives-5 --libs --cflags)

contrail: contrail.c
	$(CC) -o contrail contrail.c -ggdb -lm $(CFLAGS)

beat_circle: beat_circle.c
	$(CC) -o beat_circle beat_circle.c -ggdb -lm $(CFLAGS)

beat_square: beat_square.c
	$(CC) -o beat_square beat_square.c -ggdb -lm $(CFLAGS)