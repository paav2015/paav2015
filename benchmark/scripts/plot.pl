#!/usr/bin/perl

# options without arguments
%options = (
  help =>         { usage => 'print usage',
                    defvalue => '0' },
  debug =>        { usage => 'do not execute commands, print them instead',
                    defvalue => '0' },
);

# options with one argument
%options1 = (
  resultdir =>    { usage => 'directory with result *.out files produced by benchmark.pl',
                    defvalue => '' },
  outdir =>       { usage => 'output directory for csv and plot files',
                    defvalue => '' },
  baseline =>     { usage => 'baseline matching string => benchmarkname_<baseline>.out',
                    defvalue => '' },
  compared =>     { usage => 'compared matching sting => benchmarkname_<compared>.out (used in the \'geotable\' mode )',
                    defvalue => ' ' },
  extralabel =>   { usage => 'additional label on outfiles',
                    defvalue => ' ' },
  threadrange =>  { usage => 'threadrange "1-5"',
                    defvalue => '' },
  threadstep =>   { usage => 'threadstep',
                    defvalue => '1'},
  plotmode =>     { usage => 'plot mode (\'speedup\', \'geotable\')',
                    defvalue => 'speedup'},
);

%glelinetypes = (
  ompss => [ "color royalblue marker diamond msize 0.2", "lstyle 2 color steelblue" ],
  pthread => [ "color firebrick marker oplus msize 0.2", "lstyle 3 color orangered" ],
  other => [ "color seagreen marker cross msize 0.2", "lstyle 8 color purple marker triangle msize 0.2" ],
);

sub print_help{
  print "Usage: ./plot.pl [options]\n";
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
  if (! -e $options1{resultdir}{value}) {
    print "$options1{resultdir}{value} cannot be accessed or does not exist\n";
    exit(1);
  }
  if (! -e $options1{outdir}{value}) {
    print "$options1{outdir}{value} cannot be accessed or does not exist\n";
    exit(1);
  }

  if ($options1{threadrange}{value} =~/(\d+)-(\d+)/ ) {
    if ($1 > $2){
      print STDERR "minthreads cannot be larger than maxthread\n";
      exit(1);
    }
  }
  else {
    print STDERR "invalid threadrange ($options1{threadrange}{value}) specified\n";
    exit(1);
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

###################################################################################################

sub read_benchmark_results{
  my $benchmark = $_[0];
  my $results = $_[1];
  my $resultdir = $options1{resultdir}{value};
  my $baseline = $options1{baseline}{value};

  my @plotlinesfiles = grep { /^$benchmark\_/ } `ls $resultdir`;

  foreach (@plotlinesfiles){
    open (resfile, "<$resultdir/$_") or die $!;
    $_ =~/$benchmark\_(.*).out/.*/;
    my $id = $1;
    while (<resfile>) {
      if ( $_ =~/(\d+)\s+\|.+\|\s(\d+\.\d+)$/ ) {
        $results->{$id}{$1} = $2;
      }
    }
    close resfile;
  }
}

sub create_plotfiles{
  my $benchmark = $_[0];
  my $results = $_[1];
  my $outdir = $options1{outdir}{value};
  my $baseline = $options1{baseline}{value};

  open (csvfile, ">$outdir/$benchmark.csv") or die $!;
  print csvfile "threads";
  foreach my $plotline (sort (keys %$results)) {
    if ($plotline ne $baseline){
      my $label = $plotline; $label =~ s/\_/ /;
      print csvfile ",\t\"$label\"";
    }
  }
  print csvfile "\n";

  $options1{threadrange}{value} =~/(\d+)-(\d+)/;
  my $minthreads= $1;
  my $maxthreads= $2;
  my $threadstep= $options1{threadstep}{value};

  my $baselinetime = $results->{$baseline}{1};

  for ($t=$minthreads; $t <= $maxthreads; $t += $threadstep){
    print csvfile "$t";
    foreach my $plotline (sort (keys %$results)) {
      if ($plotline ne $baseline){
        my $speedup = $baselinetime / $results->{$plotline}{$t};
        printf csvfile ",\t%.3f", $speedup;
      }
    }
    #only effective in the first iteration to floor the t to the nearest multiple of threadstep
    $t-= $t % $threadstep;
    print csvfile "\n";
  }
  close csvfile;

  open (glefile, ">$outdir/$benchmark.gle") or die $!;
  print glefile "size 10 7\n" .
                "set texscale fixed\n" .
                "set hei 0.30\n" .
                "set font ss\n" .
                "\n" .
                "begin graph\n" .
                "  scale auto\n" .
                "  xtitle \"Threads\"\n" .
                "  ytitle \"Speedup\"\n" .
                "  xsubticks off\n" .
                "  xaxis dticks $threadstep\n" .
                "  xaxis min 0.01 max $maxthreads\n" .
                "  yaxis min 0.01 max $maxthreads\n" .
                "  key pos tl compact nobox\n" .
                "  data \"$benchmark.csv\"\n" ;

  my $ompssline=0;
  my $pthreadline=0;
  my $otherline=0;
  my $linenum=1;
  foreach my $plotline (sort (keys %$results)) {
    if ($plotline ne $baseline){
      print glefile "  d$linenum line " ; $linenum++;
      if ($plotline =~/^ompss/){
        print glefile "$glelinetypes{ompss}[$ompssline]\n";
        $ompssline++;
      } elsif ($plotline =~/^pthread/){
        print glefile "$glelinetypes{pthread}[$pthreadline]\n";
        $pthreadline++;
      } else {
        print glefile "$glelinetypes{other}[$otherline]\n";
        $otherline++;
      }
    }
  }

  print glefile "end graph\n";
  close glefile;

  system ("gle", "-device", "pdf", "$outdir/$benchmark.gle" );
}

sub calc_relperf{
  my $benchmark = $_[0];
  my $results = $_[1];
  my $relperf = $_[2];
  $options1{threadrange}{value} =~/(\d+)-(\d+)/;
  my $minthreads= $1;
  my $maxthreads= $2;
  my $threadstep= $options1{threadstep}{value};

  my $baseline = $options1{baseline}{value};
  my $compared = $options1{compared}{value};

  if (!(exists $results->{$baseline}) || !(exists $results->{$compared})){
    return;
  }

  my $geomean=1;
  my $num=0;
  for (my $t=$minthreads; $t <= $maxthreads; $t += $threadstep){
    $relperf->{$t} = $results->{$baseline}{$t}/$results->{$compared}{$t};
    $geomean*=$relperf->{$t};

    #only effective in the first iteration to floor the t to the nearest multiple of threadstep
    $t-= $t % $threadstep;
    $num++;
  }
  $geomean**= 1/$num;
  $relperf->{geomean} = $geomean;
}

@cellcolor = ("red\!40", "red\!20", "red\!10", "white", "green\!10", "green\!20", "green\!40" );
sub get_cellcolor{
  my $color;
  my $relperf = $_[0];
  if ($relperf < 0.67){
    $color= $cellcolor[0];
  } elsif ($relperf < 0.80){
    $color= $cellcolor[1];
  } elsif ($relperf < 0.91){
    $color= $cellcolor[2];
  } elsif ($relperf < 1.10){
    $color= $cellcolor[3];
  } elsif ($relperf < 1.25){
    $color= $cellcolor[4];
  } elsif ($relperf < 1.5){
    $color= $cellcolor[5];
  } else {
    $color= $cellcolor[6];
  }
  return $color;
}
sub print_geomean{
  my $relperf = $_[0];

  my $sep = " & ";
  my $endline = "\\\\\n";
  my $line = "\\hline\n";

  my $outdir = $options1{outdir}{value};
  my $baseline = $options1{baseline}{value};
  my $compared = $options1{compared}{value};
  my $extralabel = $options1{extralabel}{value};

  $options1{threadrange}{value} =~/(\d+)-(\d+)/;
  my $minthreads= $1;
  my $maxthreads= $2;
  my $threadstep= $options1{threadstep}{value};

  my $baselinetime = $results->{$baseline}{1};
  my $num=0;

  my @bench = (keys %$relperf);
  my %geomean_vertical;

  my $outfile = "$outdir/geomean\_$baseline\_$compared$extralabel.tex";
  $outfile =~s/\s+//g;
  open (texfile, ">$outfile") or die $!;

  print texfile "\\begin\{tabular\}\{\|l";
  for (my $t=$minthreads; $t <= $maxthreads; $t += $threadstep){
    print texfile "\|c";
    $t-= $t % $threadstep;
  }
  print texfile "\!\{\\vrule width 1pt\}c\|\}\n";
  print texfile $line;
  print texfile "\\rowcolor\{gray\} benchmark";
  for (my $t=$minthreads; $t <= $maxthreads; $t += $threadstep){
    print texfile $sep . $t;
    $num++;
    $geomean_vertical{$t} = 1;
    $t-= $t % $threadstep;
  }
  $geomean_vertical{geomean} = 1;
  print texfile $sep . "geomean" . $endline;

  my $numbenchmarks=0;
  foreach my $benchmark (sort (keys %$relperf)) {
    next if (!(exists $relperf->{$benchmark}{'1'}) );

    print texfile $line;
    $numbenchmarks++;
    print texfile "$benchmark";
    foreach my $t (sort { $a <=> $b} (grep {/[0-9]+/} keys %{ $relperf->{$benchmark} })) {
      my $cc = "\\cellcolor\{" . &get_cellcolor($relperf->{$benchmark}{$t}) . "\}";
      printf texfile "$sep$cc\{%.2f\}", $relperf->{$benchmark}{$t};
      $geomean_vertical{$t}*=$relperf->{$benchmark}{$t}
    }
    $geomean_vertical{geomean}*=$relperf->{$benchmark}{geomean};
    my $cc = "\\cellcolor\{" . &get_cellcolor($relperf->{$benchmark}{geomean}) . "\}";
    printf texfile "$sep$cc\{%.2f\}$endline", $relperf->{$benchmark}{geomean};
  }

  print texfile "\\noalign\{\\hrule height 1pt\}\n";
  print texfile "geomean";
  foreach my $t (sort { $a <=> $b} (grep {/[0-9]+/} keys %geomean_vertical)) {
    $geomean_vertical{$t}**=(1/$numbenchmarks);
    my $cc = "\\cellcolor\{" . &get_cellcolor($geomean_vertical{$t}) . "\}";
    printf texfile "$sep$cc\{%.2f\}", $geomean_vertical{$t};
  }
  $geomean_vertical{geomean}**=(1/$numbenchmarks);
  my $cc = "\\cellcolor\{" . &get_cellcolor($geomean_vertical{geomean}) . "\}";
  printf texfile "$sep$cc\{%.2f\}$endline", $geomean_vertical{geomean};
  print texfile $line;

  print texfile "\\end\{tabular\}\n";

  close texfile;
}

my @baselinefiles = grep { /$options1{baseline}{value}\.out$/ } `ls $options1{resultdir}{value}`;
my @benchmarks = @baselinefiles;
foreach (@benchmarks){
  $_ =~ s/\_$options1{baseline}{value}\.out//g;
}

my %results;
foreach (@benchmarks) {
  my $benchmark = $_;
  my %benchresults;

  $benchmark=~s/\R//g;
  &read_benchmark_results($benchmark, \%benchresults);
  $results{$benchmark} = \%benchresults;
}

if ($options1{plotmode}{value} eq 'speedup'){
  foreach my $benchmark (keys %results) {
    &create_plotfiles($benchmark, \%{ $results{$benchmark} });
  }
} elsif ($options1{plotmode}{value} eq 'geotable'){
  my %relperf;
  foreach my $benchmark (keys %results) {
    my %benchrelperf;
    &calc_relperf($benchmark, \%{ $results{$benchmark} }, \%benchrelperf );
    $relperf{$benchmark} = \%benchrelperf;

  }
  &print_geomean(\%relperf);
}
