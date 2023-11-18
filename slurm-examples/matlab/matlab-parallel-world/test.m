  n = 10;
  workers = 6;
  fprintf ( 1, '\n' );
  fprintf ( 1, 'HELLO_POOL\n' );
  fprintf ( 1, '  Call HELLO_FUN to say hello %d times using %d workers.\n', n, workers );

  parpool ( 'local', workers )

  tic
  hello_fun ( n );
  toc
  
  delete(gcp)

function hello_fun(x)
	for k = 0:x
		fprintf(1, '%d: HelloWorld\n', k);
	end
end
