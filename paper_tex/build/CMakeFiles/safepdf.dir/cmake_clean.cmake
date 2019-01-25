file(REMOVE_RECURSE
  "paper.aux"
  "paper.bbl"
  "paper.blg"
  "paper-blx.bib"
  "paper.glg"
  "paper.glo"
  "paper.gls"
  "paper.idx"
  "paper.ilg"
  "paper.ind"
  "paper.ist"
  "paper.log"
  "paper.out"
  "paper.toc"
  "paper.lof"
  "paper.xdy"
  "paper.synctex.gz"
  "paper.synctex.bak.gz"
  "paper.dvi"
  "paper.ps"
  "paper.pdf"
  "paper.aux"
  "paper.tex.aux"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/safepdf.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
