#
# Makefile
#

LATEXMK=latexmk

MAIN=slides

TEXFILES=$(wildcard *.tex)

all: $(TEXFILES)
	@$(LATEXMK) -pdf $(MAIN)

luatex: $(TEXFILES)
	@$(LATEXMK) -pdflatex=lualatex -pdf $(MAIN)

force:
	@$(LATEXMK) -f -pdf $(MAIN)

clean:
	@$(LATEXMK) -c ; rm *.snm *.nav

distclean: clean
	@$(LATEXMK) -C

help:
	@echo -e "Usage : make [target]\n\
	all		produce the PDF (default)\n\
	force		force compilation if possilbe\n\
	clean		clean  unnecessary files\n\
	distclean	clean deeper\n\
	help		display this help"

