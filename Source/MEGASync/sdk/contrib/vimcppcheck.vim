" vimcppcheck.vim
"  ===================================================================
"  Code Checking with cppcheck (1)
"  Based on:
"  http://stackoverflow.com/questions/19157270/vim-cppcheck-which-errorformat-to-use
"  ===================================================================

function! Cppcheck_1()
  setlocal makeprg=cppcheck\ --enable=style,information,performance,portability,missingInclude,unusedFunction\ --std=c++03\ --force\ %
  setlocal errorformat+=[%f:%l]\ ->\ %m,[%f:%l]:%m
  let curr_dir = expand('%:h')
  if curr_dir == ''
    let curr_dir = '.'
  endif
  echo curr_dir
  execute 'lcd ' . curr_dir
  execute 'make'
  execute 'lcd -'
  exe ":botright cwindow"
  :copen
endfunction


:menu Build.Code\ Checking.cppcheck :cclose<CR>:update<CR>:call Cppcheck_1() <cr>