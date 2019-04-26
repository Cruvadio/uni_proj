" Подсвечивать синтаксис
syntax on
" Указывать номера строк
set number
" Подсветить максимальную ширину строки
" Цвет линии - тёмно-серый

" С/C++ файлы
" Расставлять отступы в стиле С
autocmd filetype c,cpp set cin

" make-файлы
" В make-файлах нам не нужно заменять табуляцию пробелами
autocmd filetype make set noexpandtab
autocmd filetype make set nocin


" NERDTree
" Открывать дерево по нажаить Ctrl+n
map <c-n> :NERDTreeToggle<cr>
" Немного магии, если мы запустим Vim без указания файла для редактирования,
" то дерево будет открыто, а если будет указан файл, то дерево
" открыто не будет
autocmd StdinReadPre * let s:std_in=1
autocmd VimEnter * if argc() == 0 && !exists("s:std_in") | NERDTree | endif
" Открывать новые окна справа
set splitright
