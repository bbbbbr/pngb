# CC = gcc
# DEL = rm -f
# SRCDIR = src
# BUILDDIR = build
# OUTDIR = bin
# LODEPNGDIR = lodepng
# TILEMAPGBDIR = tilemap_gb
# INCS = -I"$(SRCDIR)" -I"$(LODEPNGDIR)" -I"$(TILEMAPGBDIR)"
# OBJS = $(BUILDDIR)/lodepng.o $(BUILDDIR)/pngb.o $(BUILDDIR)/main.o
# EXE = pngb
# CFLAGS = $(INCS)
# LFLAGS = -s

# $(EXE):	$(BUILDDIR) $(OBJS)
# 	$(CC) $(CFLAGS) -Wl,$(LFLAGS) -o $(OUTDIR)/$(EXE) $(OBJS)

# $(BUILDDIR) :
# 	mkdir $(BUILDDIR)

# $(BUILDDIR)/lodepng.o: $(LODEPNGDIR)/lodepng.c
# 	$(CC) $(CFLAGS) -c $(LODEPNGDIR)/lodepng.c -o $(BUILDDIR)/lodepng.o

# $(BUILDDIR)/pngb.o: $(SRCDIR)/pngb.c
# 	$(CC) $(CFLAGS) -c $(SRCDIR)/pngb.c -o $(BUILDDIR)/pngb.o

# $(BUILDDIR)/main.o: $(SRCDIR)/main.c
# 	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(BUILDDIR)/main.o


# clean:
# 	$(DEL) $(BUILDDIR)/*.o
# 	$(DEL) $(OUTDIR)/$(EXE)

CC = gcc
DEL = rm -f
SRCDIR = src
LODEPNGDIR = lodepng
TILEMAPGBDIR = tilemap_gb
OUTDIR = bin
INCS = -I"$(SRCDIR)" -I"$(LODEPNGDIR)" -I"$(TILEMAPGBDIR)"
CFLAGS = $(INCS)

csrc = $(wildcard src/*.c) \
		$(wildcard lodepng/*.c) \
		$(wildcard tilemap_gb/*.c)
obj = $(csrc:.c=.o)

LDFLAGS = -s

pngb: $(obj)
	$(CC) $(CFLAGS) -o $(OUTDIR)/$@ $^ $(LDFLAGS)


clean:
	rm -f $(obj) $(OUTDIR)/pngb