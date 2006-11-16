:set tags=~/.vim/tags/gloox
:set tags+=~/.vim/tags/qt4
:set tags+=../release/tags
:map <C-R> :!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q ../src<CR>

