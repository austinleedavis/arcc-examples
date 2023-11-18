% Source code here: https://raw.githubusercontent.com/pmocz/parallelMatlabExample/master/mcPI.m

%% Monte Carlo calculation of PI using pfor on a single node with up to 32 Matlab workers (cores)
% Philip Mocz (2017)
% Princeton University

%% Start the parallel pool
Nworker = str2num(getenv('SLURM_TASKS_PER_NODE'));
parpool('local', Nworker);

rng(42);   % seed for the random number generator
Ndarts = 1e7;



%% First calculate the value in serial on the Node to get a baseline
% throw a random dart in a box, count it if it fell inside the circle
tic;
count = 0;
for i = 1:Ndarts
    x = rand(1);
    y = rand(1);
    if x^2 + y^2 <= 1
        count = count + 1;
    end
end
piEstimate = 4*count/Ndarts;
elapsedTimeSerial = toc;

fprintf('PI is estimated to be %8.7f\n',piEstimate);
fprintf('Code took %8.2f seconds with single core\n', elapsedTimeSerial);



%% Next, do the parallel calculation with Nworker cores
tic;
count = 0;
parfor i = 1:Ndarts
    x = rand(1);
    y = rand(1);
    if x^2 + y^2 <= 1
        count = count + 1;
    end
end
piEstimate = 4*count/Ndarts;
elapsedTimeParallel = toc;

fprintf('PI is estimated to be %8.7f\n',piEstimate);
fprintf('Code took %8.2f seconds with %d workers\n', elapsedTimeParallel, Nworker);


%% Finally, do the calculation independently on each core
% this is an example of single program multiple data (SPMD)
fprintf ( 'Beginning spmd calculation...\n ');
spmd
    nproc = numlabs;  % get total number of workers
    iproc = labindex; % get lab ID
    if ( iproc == 1 )
        fprintf ( 1, ' Running with  %d labs.\n', nproc );
    end
    
    tic;
    count = 0;
    for i = 1:Ndarts
        x = rand(1);
        y = rand(1);
        if x^2 + y^2 <= 1
            count = count + 1;
        end
    end
    piEstimate = 4*count/Ndarts;
    elapsedTime = toc;
    
    fprintf ( 'Rank %d out of %d reports PI to be %8.7f taking %8.2f seconds \n', iproc, nproc, piEstimate, elapsedTime );
end



%% clean up
delete(gcp);
exit;



