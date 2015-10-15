#
# Makefile
#

LATEXMK=latexmk -bibtex

MAIN=analysis_ext
# Require latexdiff >= 1.1.0 to run properly
REVDIFF=HEAD

TEXFILES=$(wildcard *.tex)

all: $(TEXFILES)
	@$(LATEXMK) -pdf $(MAIN)

diff:	$(TEXFILES)
	latexdiff-vc --config="\"PICTUREENV=(?:picture|DIFnomarkup|figure|lstlisting)[\w\d*@]*\"" -t CCHANGEBAR --driver=pdftex --flatten=keep-intermediate --force --svn -r $(REVDIFF) $(MAIN).tex
	@$(LATEXMK) -pdf $(MAIN)-diff$(REVDIFF)

luatex: $(TEXFILES)
	@$(LATEXMK) -pdflatex=lualatex -pdf $(MAIN)

force:
	@$(LATEXMK) -f -pdf $(MAIN)

clean:
	@$(LATEXMK) -c

distclean: clean
	@$(LATEXMK) -C

help:
	@echo -e "Usage : make [target]\n\
	all		produce the PDF (default)\n\
	force		force compilation if possilbe\n\
	clean		clean  unnecessary files\n\
	distclean	clean deeper\n\
	help		display this help"

