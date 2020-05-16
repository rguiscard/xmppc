ADOC_FILE = xmppc.1.adoc

all:
	doxygen Doxyfile
	asciidoc -b docbook $(ADOC_FILE)
	asciidoc -b html $(ADOC_FILE)
	a2x --doctype manpage --format manpage $(ADOC_FILE)
