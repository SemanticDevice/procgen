CC=gcc
CFLAGS=$$(pkg-config allegro-5 allegro_font-5 allegro_image-5 allegro_primitives-5 --libs --cflags)
INC_DIRS=-Iperlin-noise/src -Iprocgenlib

contrail: contrail.c
	$(CC) -o contrail contrail.c perlin-noise/src/noise1234.c -ggdb -lm $(CFLAGS) $(INC_DIRS)

beat_circle: beat_circle.c
	$(CC) -o beat_circle beat_circle.c -ggdb -lm $(CFLAGS)

beat_square: beat_square.c
	$(CC) -o beat_square beat_square.c -ggdb -lm $(CFLAGS)

test: test_line_noise.c
	$(CC) -o test test_line_noise.c perlin-noise/src/noise1234.c -ggdb -lm $(CFLAGS) $(INC_DIRS)
