#
# Makefile
#

LATEXMK=latexmk

MAIN=paper

TEXFILES=$(wildcard *.tex)

all: $(TEXFILES)
	@$(LATEXMK) -pdf $(MAIN)

force:
	@$(LATEXMK) -f -pdf $(MAIN)

clean:
	@$(LATEXMK) -c ; rm -f *.out *.bbl

distclean: clean
	@$(LATEXMK) -C ; (find . -name "*~" | xargs rm -f)

help:
	@echo -e "Usage : make [target]\n\
	all		produce the PDF (default)\n\
	force		force compilation if possilbe\n\
	clean		clean  unnecessary files\n\
	distclean	clean deeper\n\
	help		display this help"

