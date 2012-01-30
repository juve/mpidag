DOCBOOK_XSL = /opt/local/share/xsl/docbook-xsl
ifeq ($(shell test -d $(DOCBOOK_XSL) || echo n),n)
$(error Set DOCBOOK_XSL to the path to your DocBook stylesheets)
endif

ASCIIDOC = asciidoc -d manpage -b docbook
XSLTPROC = xsltproc -xinclude
FOP = fop

DOCBOOK2FO = $(XSLTPROC) $(DOCBOOK_XSL)/fo/docbook.xsl
DOCBOOK2MAN = $(XSLTPROC) $(DOCBOOK_XSL)/manpages/docbook.xsl
DOCBOOK2HTML = $(XSLTPROC) $(DOCBOOK_XSL)/html/docbook.xsl

TXTFILES = $(shell ls *.txt)
MANPAGES = $(TXTFILES:.txt=.1)
XMLMANPAGES = $(TXTFILES:.txt=.xml)
PDFMANPAGES = $(TXTFILES:.txt=.pdf)
HTMLMANPAGES = $(TXTFILES:.txt=.html)

.PHONY: manpages pdfmanpages htmlmanpages

.PRECIOUS: %.xml

all : manpages pdfmanpages htmlmanpages

manpages: $(MANPAGES)
pdfmanpages: $(PDFMANPAGES)
htmlmanpages: $(HTMLMANPAGES)

%.html : %.xml
	$(DOCBOOK2HTML) $< > $@

%.1 : %.xml
	$(DOCBOOK2MAN) $<

%.pdf : %.fo
	$(FOP) $< $@

%.fo : %.xml
	$(DOCBOOK2FO) $< > $@

%.xml : %.txt
	$(ASCIIDOC) -o $@ $<
	sed -i "" -E "s/id=\"(_[^\"]+)\"/id=\"$<\1\"/" $@

clean:
	rm -f *.xml *.fo *.pdf *.1 *.html
