ProjectName Embrial

" Language-agnostic TOC generator with smart navigation and safe updates.
" Save this as lang_toc.vim and :source it.

" Helper: find headers and format their display text
function! s:GetFileHeaders() abort
    let l:headers = []
    let l:in_block_comment = 0

    for l:lnum in range(1, line('$'))
        let l:original_line = getline(l:lnum)
        let l:working_line = l:original_line

        " Handle multi-line block comments (/* ... */)
        if l:in_block_comment
            if l:working_line =~ '\*/'
                let l:working_line = matchstr(l:working_line, '^.\{-}\ze\*/')
                let l:in_block_comment = 0
            else
                " Entire line is inside a multi-line comment block
                let l:working_line = ''
                let l:in_block_comment = 1
            endif
        elseif l:working_line =~ '/\*' && l:working_line !~ '\*/'
            " Line enters a multi-line comment block; keep text after /* (if any)
            let l:working_line = matchstr(l:working_line, '/\*\zs.*$')
            let l:in_block_comment = 1
        else
            " Isolate inline single-line comments or inline block comments
            if l:working_line =~ '//'
                let l:working_line = matchstr(l:working_line, '//\zs.*$')
            elseif l:working_line =~ '/\*.\{-}\*/'
                let l:working_line = matchstr(l:working_line, '/\*\zs.\{-}\ze\*/')
            endif
        endif

        " Create Display Line: Strip outer padding spaces/tabs
        let l:display_text = substitute(l:working_line, '^\s\+\|\s\+$', '', 'g')
        let l:display_line = printf('%s %d', l:display_text, l:lnum)

        " Create Validation Line: Strip ALL whitespaces to check structure securely
        let l:validation_line = substitute(l:working_line, '\s\+', '', 'g')

        " Validate: identifiers (letters/digits/underscore) ending in a colon
        if l:validation_line =~# '^[a-zA-Z][a-zA-Z0-9_]*:$'
            if !empty(l:display_text)
                call add(l:headers, {'name': l:display_line, 'line': l:lnum})
            endif
        endif
    endfor
    return l:headers
endfunction

" Sidebar selection: pick a header and close the sidebar
function! s:SelectHeaderAndClose() abort
    let l:selected_text = getline('.')

    if empty(l:selected_text) || !has_key(b:header_map, l:selected_text)
        return
    endif

    let l:target_line = b:header_map[l:selected_text]
    let l:target_win = b:origin_win_id

    bwipeout!

    call win_gotoid(l:target_win)
    execute 'normal! ' . l:target_line . 'G'
    echo "Jumped to " . l:selected_text . ":"
endfunction

" Main entry / jump logic
function! s:HeaderJump(arg) abort
    " If the buffer contains an inserted TOC (first line is 'Table of contents:')
    if <SID>IsTOCHeader()
        let l:toc_line = getline('.')
        let l:toc_line = substitute(l:toc_line, '^\s*', '', '')

        " If the line looks like: "<name> ....   <number>"
        if l:toc_line =~# '\.\+\s*\d\+$'
            let l:toc_name = matchstr(l:toc_line, '^.\{-}\ze\s\+\.\+\s*\d\+$')
            let l:toc_name = substitute(l:toc_name, '\s\+$', '', '')

            " Find the actual header with that name using a literal prefix match
            let l:all_headers = <SID>GetFileHeaders()
            for l:header in l:all_headers
                if stridx(l:header.name, l:toc_name) == 0
                    execute 'normal! ' . l:header.line . 'G'
                    echo "Jumped to " . l:header.name . ":"
                    return
                endif
            endfor
        endif
    endif

    let l:all_headers = <SID>GetFileHeaders()
    let l:origin_win = win_getid()

    " Rule 0: numeric -> nth header (1-indexed)
    if a:arg =~# '^\d\+$'
        let l:index = str2nr(a:arg) - 1
        if l:index < 0 || l:index >= len(l:all_headers)
            echohl ErrorMsg | echo "No header numbered: " . a:arg | echohl None
            return
        endif
        execute 'normal! ' . l:all_headers[l:index]['line'] . 'G'
        echo "Jumped to " . l:all_headers[l:index]['name'] . ":"
        return
    endif

    if empty(a:arg)
        let l:matches = l:all_headers
    else
        let l:matches = filter(copy(l:all_headers), 'v:val["name"] =~# "^" . a:arg')
    endif

    if empty(l:matches)
        echohl ErrorMsg | echo "No headers found matching: " . a:arg | echohl None
        return
    endif

    " If exactly one match and the user typed something, jump instantly
    if len(l:matches) == 1 && !empty(a:arg)
        execute 'normal! ' . l:matches[0]['line'] . 'G'
        echo "Jumped to " . l:matches[0]['name'] . ":"
        return
    endif

    " Otherwise show the sidebar
    rightbelow vertical 30new
    setlocal buftype=nofile bufhidden=wipe noswapfile nobuflisted nomodifiable
    setlocal cursorline number

    let b:header_map = {}
    let b:origin_win_id = l:origin_win
    let l:lines_to_write = []

    for l:item in l:matches
        call add(l:lines_to_write, l:item['name'])
        let b:header_map[l:item['name']] = l:item['line']
    endfor

    setlocal modifiable
    call setline(1, l:lines_to_write)
    setlocal nomodifiable

    nnoremap <buffer> <CR> :call <SID>SelectHeaderAndClose()<CR>
    nnoremap <buffer> q :bwipeout!<CR>

    if empty(a:arg)
        echo "Showing all headers. Choose one and press Enter."
    else
        echo "Ambiguous match! Choose from the filtered list and press Enter."
    endif
endfunction

" Return the last line number of an inline TOC (0 = no inline TOC).
function! s:TOCHeaderEnd() abort
  if line('$') < 1
    return 0
  endif

  let l:first = getline(1)
  if type(l:first) != type('') " safety
    return 0
  endif

  let l:first = substitute(l:first, '^\ufeff', '', '')    " strip BOM
  let l:first = substitute(l:first, '^\s\+\|\s\+$', '', 'g')

  if tolower(l:first) !=# 'table of contents:'
    return 0
  endif

  " Scan subsequent lines; accept lines that look like TOC lines:
  "  - wrapped content lines start with indentation (e.g. 4 spaces)
  "  - number lines have dots and end with a number (e.g. " ...  123")
  " Stop when we find the first line that does NOT match either form.
  for l:lnum in range(2, line('$'))
    let l:ln = substitute(getline(l:lnum), '^\ufeff', '', '')

    " Accept indented wrapped lines (starts with at least one space/tab then non-space)
    if l:ln =~# '^\s\+\S'
      continue
    endif

    " Accept "number" lines: contains at least one dot before the trailing number
    if l:ln =~# '\.\+\s*\d\+$'
      continue
    endif

    " Otherwise this line is not part of the TOC -> the TOC ends on the previous line.
    return l:lnum - 1
  endfor

  " No non-TOC line found: treat the rest of the buffer as the TOC.
  return line('$')
endfunction

" Keep the existing IsTOCHeader API but implement it in terms of TOCHeaderEnd.
function! s:IsTOCHeader() abort
  return <SID>TOCHeaderEnd() !=# 0
endfunction

" Format the headers into the pretty TOC representation (supports wrapping).
function! s:FormatTOC(headers) abort
    let l:toc_width = 80
    let l:indent = 4

    " Width of the number column = longest number + 1.
    let l:max_line = 0
    for l:header in a:headers
        let l:max_line = max([l:max_line, l:header.line])
    endfor
    let l:number_width = strlen(string(l:max_line)) + 1

    let l:content_width = l:toc_width - l:indent - 1 - l:number_width
    if l:content_width < 10
        let l:content_width = 10
    endif

    let l:result = []
    for l:header in a:headers
        let l:fullname = substitute(l:header.name, '\s\+\d\+$', '', '')
        let l:fullname = substitute(l:fullname, '\s\+$', '', '')

        " Split into chunks of content_width characters (simple split, not word-wrap)
        let l:chunks = []
        let l:pos = 0
        while l:pos < strlen(l:fullname)
            call add(l:chunks, strpart(l:fullname, l:pos, l:content_width))
            let l:pos += l:content_width
        endwhile

        " If fits in one chunk, try to render on a single line with dots
        if len(l:chunks) == 1
            let l:name = l:chunks[0]
            let l:dot_space = l:toc_width - l:indent - strlen(l:name) - 1 - l:number_width
            if l:dot_space >= 1
                let l:line =
                            \ repeat(' ', l:indent)
                            \ . l:name
                            \ . ' '
                            \ . repeat('.', l:dot_space)
                            \ . printf('%*d', l:number_width, l:header.line)
                call add(l:result, l:line)
                continue
            endif
        endif

        " Multi-line: emit all chunks except last, then final chunk + dots + number
        for l:i in range(0, len(l:chunks) - 2)
            call add(l:result, repeat(' ', l:indent) . l:chunks[l:i])
        endfor
        if len(l:chunks) >= 1
            let l:last_chunk = l:chunks[len(l:chunks) - 1]
            let l:dot_space = l:toc_width - l:indent - strlen(l:last_chunk) - 1 - l:number_width
            if l:dot_space < 1
                let l:dot_space = 1
            endif
            let l:number_line =
                        \ repeat(' ', l:indent)
                        \ . l:last_chunk
                        \ . ' '
                        \ . repeat('.', l:dot_space)
                        \ . printf('%*d', l:number_width, l:header.line)
            call add(l:result, l:number_line)
        endif
    endfor

    return l:result
endfunction

" Safe removal of TOC: only delete an actual block; guard against backwards ranges.
function! s:RemoveTOC() abort
    if !<SID>IsTOCHeader()
        return 0
    endif

    let l:end = 0
    for l:lnum in range(2, line('$'))
        if getline(l:lnum) =~# '^\s*$'
            let l:end = l:lnum
            break
        endif
    endfor

    if l:end > 0
        let l:start = 2
        let l:last = l:end - 1
        if l:last >= l:start
            execute l:start . ',' . l:last . 'delete _'
        endif
    else
        if line('$') > 1
            execute '2,$delete _'
        endif
    endif
    return 1
endfunction

" Compute a stable hash for a TOC (sha1 if avaiable otherwise fallback to fast
" deterministic rolling-hash implemented in Vimscript otherwise
function! s:ComputeTOCHash(toc_lines) abort
    " Join the TOC into a single string
    let l:txt = join(a:toc_lines, "\n")

    " Prefer the builtin sha1() if present
    if exists('*sha1')
        return sha1(l:txt)
    endif

    " Fallback: simple rolling 32-bit hash -> hex string
    if l:txt ==# ''
        return '00000000'
    endif

    let l:hash = 0
    for l:i in range(0, len(l:txt) - 1)
        let l:ch = char2nr(strpart(l:txt, l:i, 1))
        let l:hash = (l:hash * 31 + l:ch) % 0x100000000
    endfor
    return printf('%08x', l:hash)
endfunction

" Update the TOC only if it differs (or if forced).
function! s:UpdateTOC(force) abort
    if !<SID>IsTOCHeader()
        return
    endif

    let l:headers = <SID>GetFileHeaders()
    let l:toc = <SID>FormatTOC(l:headers)
    let l:new_hash = <SID>ComputeTOCHash(l:toc)

    if a:force || !exists('b:lang_toc_hash') || b:lang_toc_hash !=# l:new_hash
        call <SID>RemoveTOC()
        " Insert TOC (no leading blank, keep trailing blank).
        call append(1, l:toc)
        let b:lang_toc_hash = l:new_hash
    endif
endfunction

" Backwards-compatible GenerateTOC command (force regenerate)
function! s:GenerateTOC() abort
    call <SID>UpdateTOC(1)
endfunction

" Find a visible TOC window (search current tab's windows); return wininfo or 0.
function! s:FindTOCWindow() abort
    for wininfo in getwininfo()
        if has_key(wininfo, 'bufnr') && wininfo.bufnr > 0
            let l:lines = getbufline(wininfo.bufnr, 1, 1)
            if len(l:lines) >= 1
                let l:first = substitute(l:lines[0], '^\ufeff', '', '')
                let l:first = substitute(l:first, '^\s\+\|\s\+$', '', 'g')
                if tolower(l:first) ==# 'table of contents:'
                    return wininfo
                endif
            endif
        endif
    endfor
    return 0
endfunction

" From a header location in a normal buffer, jump to the TOC entry if possible.
function! s:JumpToTOCFromHeader() abort
    let l:lnum = line('.')
    let l:target = string(l:lnum)

    " 1) Prefer a visible TOC window (other split/window)
    let l:wininfo = <SID>FindTOCWindow()
    if l:wininfo !=# 0
        let l:bufnr = l:wininfo.bufnr
        let l:lines = getbufline(l:bufnr, 1, '$')
        let l:match_idx = -1
        for i in range(0, len(l:lines) - 1)
            if matchstr(l:lines[i], '\d\+$') ==# l:target
                let l:match_idx = i + 1
                break
            endif
        endfor
        if l:match_idx != -1
            " find top of entry block
            let l:start = l:match_idx
            while l:start > 1
                let l:prev = l:lines[l:start - 2]
                if l:prev =~# '\d\+$'
                    break
                endif
                if l:prev =~# '^\s'
                    let l:start -= 1
                    continue
                endif
                break
            endwhile
            " go to that window and position
            call win_gotoid(l:wininfo.winid)
            call cursor(l:start, 1)
            normal! zz
            return 1
        endif
    endif

    " 2) If the current buffer itself contains the TOC at the top, search here
    if <SID>IsTOCHeader()
        let l:lines = getline(1, '$')
        let l:match_idx = -1
        for i in range(0, len(l:lines) - 1)
            if matchstr(l:lines[i], '\d\+$') ==# l:target
                let l:match_idx = i + 1
                break
            endif
        endfor
        if l:match_idx != -1
            " find top of entry block
            let l:start = l:match_idx
            while l:start > 1
                let l:prev = l:lines[l:start - 2]
                if l:prev =~# '\d\+$'
                    break
                endif
                if l:prev =~# '^\s'
                    let l:start -= 1
                    continue
                endif
                break
            endwhile
            " place cursor in the current window at the TOC entry
            call cursor(l:start, 1)
            normal! zz
            return 1
        endif
    endif

    " Not found in visible TOC or inline TOC
    return 0
endfunction

" Smart Ctrl-J: update TOC from buffer, then jump to its entry (visible TOC or inline),
" otherwise open a TOC split and try again, fallback to :Header prompt.
function! s:SmartHeaderJump() abort
    " If the buffer has an inline TOC at the top AND the cursor is inside that TOC block,
    " then run the TOC-specific jump behavior. Otherwise fall through to normal behavior.
    let l:toc_end = <SID>TOCHeaderEnd()
    if l:toc_end > 0 && line('.') <= l:toc_end
        echom 'In the table of contents'
        let l:first = substitute(getline('.'), '^\s*', '', '')
        if l:first =~# '\.\+\s*\d\+$'
            call <SID>HeaderJump('')
            return
        endif

        " If within a wrapped entry, search a few lines forward for the number line.
        for off in range(1, 8)
            if line('.') + off <= line('$')
                if substitute(getline(line('.') + off), '^\s*', '', '') =~# '\.\+\s*\d\+$'
                    let l:save = getpos('.')
                    call cursor(line('.') + off, 1)
                    call <SID>HeaderJump('')
                    call setpos('.', l:save)
                    return
                endif
            endif
        endfor
        
        call feedkeys(":Header ", 'n')
        return
    endif

    echom 'Not in the table of contents'

    " Not in TOC: force-update TOC from current buffer contents (picks up unsaved edits).
    silent! call <SID>UpdateTOC(0)

    " Try to jump to an existing TOC entry (visible window or inline)
    if <SID>JumpToTOCFromHeader()
        return
    endif

    " No visible/inline TOC entry found: open a TOC view (split), update, then try again.
    execute 'rightbelow vertical 30 sbuffer %'
    call cursor(1, 1)
    silent! call <SID>UpdateTOC(1)

    if <SID>JumpToTOCFromHeader()
        return
    endif

    " Final fallback: interactive Header prompt
    call feedkeys(":Header ", 'n')
endfunction

" --- LangTOC wrapper: ensure TOC is current, then jump ---
if !exists('*LangTOC_SmartHeaderJump')
  function! LangTOC_SmartHeaderJump(...) abort
    " DEBUG: trace entry and current line
    " echom '>>> LangTOC_SmartHeaderJump: called'
    " echom '    Buf: ' . expand('%:p') . '  line#: ' . line('.') . '  text: ' . substitute(getline('.'), '\n', '', 'g')

    " Ensure the TOC is generated/updated first (so jumps reflect new headers)
    silent! call <SID>UpdateTOC(0)

    " Then perform the original smart jump
    silent! call <SID>SmartHeaderJump()

    " echom '<<< LangTOC_SmartHeaderJump: returned'
  endfunction
endif
"if !exists('*LangTOC_SmartHeaderJump')
"  function! LangTOC_SmartHeaderJump(...) abort
"    call <SID>SmartHeaderJump()
"  endfunction
"endif

nnoremap <silent> <C-J> :silent! call <SID>SmartHeaderJump()<CR>
" autocmd VimEnter * nnoremap <silent> <C-J> :call LangTOC_SmartHeaderJump()<CR>
" Map Ctrl-J to smart helper (script-local)
" nnoremap <silent> <C-J> :call <SID>SmartHeaderJump()<CR>

" Autocommands: recalc on read/enter/write/leave. Use <SID> so the calls resolve.
augroup LangTOC
    autocmd!
    autocmd BufReadPost * silent! call <SID>UpdateTOC(0)
    autocmd BufWinEnter * silent! call <SID>UpdateTOC(0)
    autocmd BufWritePre * silent! call <SID>UpdateTOC(0)
    autocmd BufWinLeave * silent! call <SID>UpdateTOC(0)
augroup END

" Expose interactive commands (defined inside the script so <SID> expands).
command! -nargs=? Header call <SID>HeaderJump(<q-args>)
command! GenerateTOC call <SID>GenerateTOC()
