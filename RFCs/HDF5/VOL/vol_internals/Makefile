filename=main

all:
	pdflatex ${filename}
	bibtex ${filename}||true
	pdflatex ${filename}
	pdflatex ${filename}

clean:
	rm -f ${filename}.{pdf,log,aux,out,dvi,bbl,blg}