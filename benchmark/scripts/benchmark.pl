#!/usr/bin/perl

use warnings;
use Cwd;
use File::Find;
use File::Basename;
use POSIX;
use List::Util qw[min max];
use feature qw(switch);
use IPC::Open3;

# options without arguments
%options = (
  help =>         { usage => 'print usage',
                    defvalue => '0' },
  debug =>        { usage => 'do not execute benchmark commands, print them instead',
                    defvalue => '0' },
  pinning =>      { usage => 'for pthreads use threadpinning option of benchmarks',
                    defvalue => '0' },
  iterinc =>      { usage => 'add "floor(sqrt(threads)-1)" to the base iteration count',
                    defvalue => '0' },
  numactl =>      { usage => 'enables numactl options (currently only meminterleave)',
                    defvalue => '0' },
);

# options with one argument
%options1 = (
  bindir =>       { usage => 'binary directory',
                    defvalue => '' },
  benchmark_pattern =>  { usage => 'grep pattern on benchmark directory',
                    defvalue => '.' },
  inputdir =>     { usage => 'input file directory',
                    defvalue => '' },
  outdir =>       { usage => 'output directory for result files',
                    defvalue => '' },
  extralabel =>   { usage => 'additional label on outfiles',
                    defvalue => ' ' },
  execmode =>     { usage => 'execution mode "seq, pthread, ompss"',
                    defvalue => 'seq' },
  minthreads =>   { usage => 'min thread count',
                    defvalue => '1' },
  maxthreads =>   { usage => 'max thread count',
                    defvalue => '1' },
  threadstep =>   { usage => 'execute only for multiple of thread step inside the core range in addition of the minimum threadvalue',
                    defvalue => '1'},
  iterations =>   { usage => 'iteration count for individual runs',
                    defvalue => '3' },
  meminterleave =>{ usage => 'use numactl to interleave the memory pages',
                    defvalue => ' ' },
);

sub print_help{
  print "Usage: ./benchmark.pl [options]\n";
  print "Possible options:\n";

  foreach $opt (keys %options) {
    printf "%-15s", "--$opt";
    print " : $options{$opt}{usage}.\n"
  }

  foreach $opt (keys %options1) {
    printf "%-15s", "--$opt";
    print " : $options1{$opt}{usage}. Default value ($options1{$opt}{defvalue})";
    print "\n";
  }
}

sub setdefaultvalues{
  foreach my $opt (keys %options) {
    $options{$opt}{value} = $options{$opt}{defvalue};
  }
  foreach my $opt (keys %options1) {
    $options1{$opt}{value} = $options1{$opt}{defvalue};
  }
}

sub checkparams{
  if ( $options{help}{value} ){
    &print_help;
    exit(0);
  }

  foreach $opt (keys %options1) {
    if ( $options1{$opt}{value} eq '' ) {
      print "--$opt not specified\n";
      exit(1);
    }
  }
  if (! -e $options1{inputdir}{value}) {
    print "$options1{inputdir}{value} cannot be accessed or does not exist\n";
    exit(1);
  }
  if (! -e $options1{outdir}{value}) {
    print "$options1{outdir}{value} cannot be accessed or does not exist\n";
    exit(1);
  }
  if (! -e $options1{bindir}{value}) {
    print "$options1{bindir}{value} cannot be accessed or does not exist\n";
    exit(1);
  }
  if ($options1{minthreads}{value} < 1) {
    print "minthreads must be larger than 0\n";
    exit(1);
  }
  if ($options1{minthreads}{value} > $options1{maxthreads}{value}) {
    print "minthreads ($options1{minthreads}{value}) > maxthreads ($options1{maxthreads}{value})\n";
    exit(1);
  }

  given($options1{execmode}{value}){
    when ('seq')     {}
    when ('pthread'){}
    when ('ompss')   {}
    default {
      print "$options1{execmode}{value} is not a valid execution mode\n";
      exit(1);
    }
  }
}

if ( $#ARGV < 1 ){
  &print_help;
  exit(1);
}

&setdefaultvalues;

while ( $#ARGV + 1 ){
  if ($ARGV[0]=~/^--/){
    my $opt = $ARGV[0];
    $opt =~ s/--//g;
    if ( exists $options{$opt} ) {
      $options{$opt}{value}=1;
    } elsif ( exists $options1{$opt} ) {
      shift;
      $options1{$opt}{value}=$ARGV[0];
    } else {
      print "Warning: unknown option $ARGV[0]\n"
    }
  }
  shift;
}

&checkparams;


#############################################################################################################

%benchmarks = (
  bodytrack => {
    cmd_args     => 'INPUTDIR/bodytrack/sequenceB_261/ 4 261 4000 5 1',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '%d %d',
  },
  'c-ray-mt'  => {
    cmd_args     => '-i INPUTDIR/c-ray/sphfract -o /dev/null -s 1920x1080 -r 2',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '-t %d -p %d',
  },
  h264dec   => {
    cmd_args     => '-i INPUTDIR/h264dec/park_joy_2160px2.h264',
    ompss_args   => '-z 8 8 --static-3d -e THREADCOUNT',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '-t %d -p %d',
  },
  kmeans    => {
    cmd_args     => '-i INPUTDIR/kmeans/edge -b -n 2000',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '-t %d -p %d',
  },
  md5       => {
    cmd_args     => '-i 7 -c 10',
    time_pattern => 'Time:\s+(\d+\.\d+)',
    thread_format=> '-t %d -p %d',
  },
  'ray-rot'   => {
    cmd_args     => 'INPUTDIR/c-ray/sphfract /dev/null 50 1920 1080 1',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '%d %d',
  },
  rgbyuv    => {
    cmd_args     => '-i INPUTDIR/rotate_rgbyuv_workloads/libppm/Berlin_Botanischer-Garten_HB_02.ppm -c 10',
    time_pattern => 'Time:\s+(\d+\.\d+)',
    thread_format=> '-t %d -p %d',
  },
  rot       => {
    cmd_args     => 'INPUTDIR/rotate_rgbyuv_workloads/libppm/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50',
    time_pattern => 'Compute:\s+(\d+\.\d+)s',
    thread_format=> '%d %d',
  },
  'rot-cc'    => {
    cmd_args     => 'INPUTDIR/rotate_rgbyuv_workloads/libppm/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50',
    time_pattern => 'Total:\s+(\d+\.\d+)s',
    thread_format=> '%d %d',
  },
  streamcluster => {
    cmd_args     => '10 20 128 200000 200000 5000 none output.txt',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '%d %d',
  },
  tinyjpeg  => {
    cmd_args     => '--benchmark INPUTDIR/tinyjpeg/earth-marker.jpg /dev/null',
    time_pattern => 'real\s+(\d+\.\d+)',
    thread_format=> '%d %d',
  },
);

sub launch_seq{
  my $bin = $_[0];
  my $cmd = $_[1];
  my $args = $benchmarks{$bin}{cmd_args};
  $args =~s/INPUTDIR/$options1{inputdir}{value}/g;

  my $command= "$cmd /usr/bin/time -p $options1{bindir}{value}/$bin $args";

  if ($options{debug}{value}){
    print "$command\n";
  } else {
    my($wtr, $rdr);
    my $pid = open3( $wtr, $rdr, 0, $command );
    waitpid($pid,0);
    local $/;
    return <$rdr>;
  }
}

sub launch_pthread{
  my $bin = $_[0];
  my $cmd = $_[1];
  my $threadcount = $_[2];
  my $pinthreads = $_[3];
  my $args = $benchmarks{$bin}{cmd_args};
  $args =~s/INPUTDIR/$options1{inputdir}{value}/g;
  my $command= "$cmd /usr/bin/time -p $options1{bindir}{value}/$bin $args";
  $command .= sprintf( " $benchmarks{$bin}{thread_format}", $threadcount, $pinthreads);

  if ($options{debug}{value}){
    print "$command\n";
  } else {
    my($wtr, $rdr);
    my $pid = open3( $wtr, $rdr, 0, $command );
    waitpid($pid,0);
    local $/;
    return <$rdr>;
  }
}

sub launch_ompss{
  my $bin = $_[0];
  my $cmd = $_[1];
  my $threadcount = $_[2];
  my $args = $benchmarks{$bin}{cmd_args};
  $args .= exists $benchmarks{$bin}{ompss_args} ? ' '.$benchmarks{$bin}{ompss_args} : '';
  $args =~s/INPUTDIR/$options1{inputdir}{value}/g;
  $args =~s/THREADCOUNT/$threadcount/g;
  my $env = "NX_PES=$threadcount";
  $env .= exists $benchmarks{$bin}{ompss_env} ? ' '.$benchmarks{$bin}{ompss_env} : '';
  my $command= "$env $cmd /usr/bin/time -p $options1{bindir}{value}/$bin $args";

  if ($options{debug}{value}){
    print "$command\n";
  } else {
    my($wtr, $rdr);
    my $pid = open3( $wtr, $rdr, 0, $command );
    waitpid($pid,0);
    local $/;
    return <$rdr>;
  }
}

sub launch_benchmark{
  my $bin = $_[0];
  my $minthreads = $options1{minthreads}{value};
  my $maxthreads = $options1{maxthreads}{value};
  my $threadstep = $options1{threadstep}{value};
  my $baseiters = $options1{iterations}{value};
  my $iterinc = $options{iterinc}{value};
  my $execmode = $options1{execmode}{value};
  my $pattern = $benchmarks{$bin}{time_pattern};
  my $pinning = $options{pinning}{value};
  my $outdir = $options1{outdir}{value};
  my $extralabel = $options1{extralabel}{value};

  print "launching benchmark $bin\n";

  my $outfile = "$bin\_$execmode$extralabel.out";
  $outfile =~s/\s+//g;
  if ($execmode eq 'pthread' && $pinning){
    $outfile = "$bin\_$execmode\_pinned.out";
  }
  print "$outdir/$outfile\n";

  my $numacmd= "";
  if ( $options{numactl}{value}) {
    $numacmd= "numactl";
    if ( $options1{meminterleave}{value} ne ' '){
      $numacmd .= " --interleave $options1{meminterleave}{value}";
    }
  }

  if (!$options{debug}{value}){
    open (out_fh, ">$outdir/$outfile") or die "Can't create $outdir/$outfile";
  }

  for (my $t=$minthreads; $t<=$maxthreads; $t+=$threadstep){
    if (!$options{debug}{value}){
      print out_fh "$t |\t";
    }
    my $totaltime = 0;
    my $nresults = 0;
    my $iters = $baseiters + ($iterinc ? floor(sqrt($t)-1): 0);

    for (my $i=0; $i<$iters; $i++){
      my $output;
      given($execmode){
        when ('seq')     {$output = &launch_seq($bin, $numacmd);}
        when ('pthread') {$output = &launch_pthread($bin, $numacmd, $t, $pinning);}
        when ('ompss')   {$output = &launch_ompss($bin, $numacmd, $t);}
        default {
          print "this should not be reachable...\n";
          exit(1);
        }
      }
      if (!$options{debug}{value}){
        if ($output =~/$pattern/){
          $totaltime += $1;
          print out_fh "$1\t";
          $nresults++; #use nresults instead of iter in case pattern does not match due to output corruption
        } else {
          printf STDERR "execution time not found in output\n";
        }
      }
    }

    if (!$options{debug}{value}){
      my $avgtime = $totaltime/$nresults;
      printf out_fh "| %.3f\n", $avgtime;
    }

    #only effective in the first iteration to floor the t to the nearest multiple of threadstep
    $t-= $t % $threadstep;
  }

  if (!$options{debug}{value}){
    close out_fh;
  }
}

if ( $options1{execmode}{value} eq 'seq' ){
  $options1{minthreads}{value} = 1;
  $options1{maxthreads}{value} = 1;
  $options1{threadstep}{value} = 1;
}

my @bins = grep { /$options1{benchmark_pattern}{value}/ } `ls $options1{bindir}{value}`;
foreach (@bins) {
  my $bin = $_;
  $bin=~s/\R//g;
  if ( exists $benchmarks{$bin}) {
    &launch_benchmark($bin);
  }
}

