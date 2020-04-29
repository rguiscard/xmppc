TXT_FILE = xmppc.1.txt

all:
	doxygen Doxyfile
	asciidoc -b docbook $(TXT_FILE)
	asciidoc -b html $(TXT_FILE)
	a2x --doctype manpage --format manpage $(TXT_FILE)
