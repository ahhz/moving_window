xsltproc --nonet --xinclude %BOOST%/tools/boostbook/xsl/docbook.xsl source/moving_window.xml > temp.xml
xsltproc --nonet --xinclude %BOOST%/tools/boostbook/xsl/html.xsl boostbook/db2html.xsl temp.xml
md html
copy index.html html\index.html
copy HTML.manifest html\HTML.manifest
copy boostbook\boostbook.css html\boostbook.css
copy boostbook\pre-boost.jpg html\pre-boost.jpg
xcopy index html\index\ /E /Y
xcopy images html\images\ /E /Y
del index.html
del HTML.manifest
del temp.xml
del index\*.* /Q
rmdir index


