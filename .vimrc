map <c-n> :NERDTreeToggle<cr>

autocmd StdinReadPre * let s:std_in=1
autocmd VimEnter * if argc() == 0 && exists("s:std_in") | NERDTree | endif

set splitright
