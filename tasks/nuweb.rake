# Rake tasks to build nuweb code (tangle) and documentation (weave)
# Source nuweb code must be in .w files in directory source.
# Any files is subdirectories of source/verbatim are copied to
# correpsonding directories in the project root directory.
# source subdirectories
namespace :nuweb do

  NUWEB_PRODUCTS = []
  NUWEB_SOURCES = Dir['source/**/*.w']
  GENERATED_DIRS = ['source/pdf']

  NUWEB_SOURCES.each do |input_fn|
    products = []
    File.open(input_fn) do |input|
      meta = '@'
      input.each_line do |line|
        if /^\s*%#{meta}r(.)%/.match(line)
          meta = $1
        elsif /^\s*[^#{meta}]?#{meta}(?:o|O)\s*(\S.*)\s*$/.match(line)
          products << $1
        end
      end
    end
    NUWEB_PRODUCTS.concat products
    products.each do |product|
      file product => [input_fn] do |t|
        puts "nuweb -t #{input_fn}"
        puts `nuweb -t #{input_fn}`
        touch product
      end
    end
  end

  Dir['source/verbatim/*'].each do |dir_path|
    next unless File.directory?(dir_path)
    dir = File.basename(dir_path)
    GENERATED_DIRS << dir
    sources = Dir["#{dir_path}/**/*"]
    NUWEB_SOURCES.concat sources
    NUWEB_PRODUCTS.concat sources.map{|s| s.sub("source/verbatim/#{dir}/","#{dir}/")}
    rule(/\A#{dir}\/.*/ =>[proc{|tn| tn.sub(/\A#{dir}\//, "source/verbatim/#{dir}/") }])  do |t|
      cp t.source, t.name  if t.source
    end
  end

  desc "Generate code from nuweb source"
  task :tangle => NUWEB_PRODUCTS + [:test]
  
  clean_exts = ['*.tex','*.dvi','*.log','*.aux','*.out']
  clobber_exts = []

  desc "Remove all nuweb generated files"
  task :clobber=>['^clobber'] do |t|
    GENERATED_DIRS.map{|dir| Dir["#{dir}/**/*"]}.flatten.each do |fn|
      rm fn unless File.directory?(fn)
    end
  end

  desc "Clean up nuweb weave temporary files"
  task :clean do |t|
    rm_r clean_exts.collect{|x| Dir.glob('source/*'+x)+Dir.glob('source/pdf/*'+x)}.flatten
  end

  desc "Generate nuweb source code documentation"
  task :weave => ['source/pdf'] + Dir['source/*.w'].collect{|fn| fn.gsub(/\.w/,'.pdf').gsub('source/','source/pdf/')}

  def rem_ext(fn, ext)
    ext = File.extname(fn) unless fn
    File.join(File.dirname(fn),File.basename(fn,ext))
  end

  def sub_dir(dir, fn)
    d,n = File.split(fn)
    File.join(d,File.join(dir,n))
  end

  def rep_dir(dir, fn)
    File.join(dir, File.basename(fn))
  end

  #note that if latex is run from the base directory and the file is in a subdirectory (source)
  # .aux/.out/.log files are created in the subdirectory and won't be found by the second
  # pass of latex;
  def w_to_pdf(s)
    fn = rem_ext(s,'.w')
    puts "dir: #{File.dirname(fn)}"
    doc_dir = File.dirname(fn)!='.' ? './pdf' : '../source/pdf'
    cd(File.dirname(fn)) do
      fn = File.basename(fn)
      2.times do
        puts "nuweb -o -l #{fn}.w"
        puts `nuweb -o -l #{fn}.w`
        puts "latex -halt-on-error #{fn}.tex"
        puts `latex -halt-on-error #{fn}.tex`
        puts "dvipdfm -o #{rep_dir(doc_dir,fn)}.pdf #{fn}.dvi"
       puts `dvipdfm -o #{rep_dir(doc_dir,fn)}.pdf #{fn}.dvi`
      end
    end
  end

  rule '.pdf' => [proc{|tn| File.join('source',File.basename(tn,'.pdf')+'.w')}] do |t|
    w_to_pdf t.source
  end

end

desc 'Generate code and documentation from nuweb sources'
task :nuweb => ['nuweb:tangle', 'nuweb:weave']
