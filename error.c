static void _GL_ATTRIBUTE_FORMAT_PRINTF (3, 0) _GL_ARG_NONNULL ((3))
error_tail (int status, int errnum, const char *message, va_list args)
{
#if _LIBC
  if (_IO_fwide (stderr, 0) > 0)
    {
      size_t len = strlen (message) + 1;
      wchar_t *wmessage = NULL;
      mbstate_t st;
      size_t res;
      const char *tmp;
      bool use_malloc = false;

      while (1)
	{
	  if (__libc_use_alloca (len * sizeof (wchar_t)))
	    wmessage = (wchar_t *) alloca (len * sizeof (wchar_t));
	  else
	    {
	      if (!use_malloc)
		wmessage = NULL;

	      wchar_t *p = (wchar_t *) realloc (wmessage,
						len * sizeof (wchar_t));
	      if (p == NULL)
		{
		  free (wmessage);
		  fputws_unlocked (L"out of memory\n", stderr);
		  return;
		}
	      wmessage = p;
	      use_malloc = true;
	    }

	  memset (&st, '\0', sizeof (st));
	  tmp = message;

	  res = mbsrtowcs (wmessage, &tmp, len, &st);
	  if (res != len)
	    break;

	  if (__builtin_expect (len >= SIZE_MAX / sizeof (wchar_t) / 2, 0))
	    {
	      /* This really should not happen if everything is fine.  */
	      res = (size_t) -1;
	      break;
	    }

	  len *= 2;
	}

      if (res == (size_t) -1)
	{
	  /* The string cannot be converted.  */
	  if (use_malloc)
	    {
	      free (wmessage);
	      use_malloc = false;
	    }
	  wmessage = (wchar_t *) L"???";
	}

      __vfwprintf (stderr, wmessage, args);

      if (use_malloc)
	free (wmessage);
    }
  else
#endif
    vfprintf (stderr, message, args);
  va_end (args);

  ++error_message_count;
  if (errnum)
    print_errno_message (errnum);
#if _LIBC
  __fxprintf (NULL, "\n");
#else
  putc ('\n', stderr);
#endif
  fflush (stderr);
  if (status)
    exit (status);
}


/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

// #if defined _LIBC && defined __libc_ptf_call
//   /* We do not want this call to be cut short by a thread
//      cancellation.  Therefore disable cancellation for now.  */
//   int state = PTHREAD_CANCEL_ENABLE;
//   __libc_ptf_call (__pthread_setcancelstate,
// 		   (PTHREAD_CANCEL_DISABLE, &state), 0);
// #endif

  flush_stdout ();
#ifdef _LIBC
  _IO_flockfile (stderr);
#endif
  if (error_print_progname)
    (*error_print_progname) ();
  else
    {
      fprintf (stderr, "%s: ", program_name);
    }

  va_start (args, message);
  error_tail (status, errnum, message, args);

#ifdef _LIBC
  _IO_funlockfile (stderr);
# ifdef __libc_ptf_call
  __libc_ptf_call (__pthread_setcancelstate, (state, NULL), 0);
# endif
#endif
}
