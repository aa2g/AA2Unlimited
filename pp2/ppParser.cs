using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.ComponentModel;
using SB3Utility;
using System.Linq;

namespace SB3Utility
{
	public class ppParser
	{
		public string FilePath { get; protected set; }
		public List<IWriteFile> Subfiles { get; protected set; }

		private string destPath;
		private bool keepBackup;

        public void ReloadSubfiles()
        {
            if (File.Exists(this.FilePath))
            {
                this.Subfiles = ppHeader.ReadHeader(this.FilePath);
            }
            else
            {
                this.Subfiles = new List<IWriteFile>();
            }
            
        }

		public ppParser(string path)
		{
			this.FilePath = path;
            ReloadSubfiles();
		}

		public BackgroundWorker WriteArchive(string destPath, bool keepBackup)
		{
			this.destPath = destPath;
			this.keepBackup = keepBackup;

			BackgroundWorker worker = new BackgroundWorker();
			worker.WorkerSupportsCancellation = true;
			worker.WorkerReportsProgress = true;

			worker.DoWork += new DoWorkEventHandler(writeArchiveWorker_DoWork);
			return worker;
		}

		void writeArchiveWorker_DoWork(object sender, DoWorkEventArgs e)
		{
			BackgroundWorker worker = (BackgroundWorker)sender;
			string backup = null;

			string dirName = Path.GetDirectoryName(destPath);
			if (dirName == String.Empty)
			{
				dirName = @".\";
			}
			DirectoryInfo dir = new DirectoryInfo(dirName);
			if (!dir.Exists)
			{
				dir.Create();
			}

			if (File.Exists(destPath))
			{
				backup = Utility.GetDestFile(dir, Path.GetFileNameWithoutExtension(destPath), ".bak");
				File.Move(destPath, backup);

				if (destPath.Equals(this.FilePath, StringComparison.InvariantCultureIgnoreCase))
					foreach (IWriteFile iw in Subfiles)
                        if (iw is ppSubfile)
                        {
                            ppSubfile subfile = iw as ppSubfile;
                            if (subfile.ppPath.Equals(this.FilePath, StringComparison.InvariantCultureIgnoreCase))
                                subfile.ppPath = backup;
                        }
			}

			try
			{
				using (BinaryWriter writer = new BinaryWriter(File.Create(destPath)))
				{
					writer.BaseStream.Seek(ppHeader.HeaderSize(Subfiles.Count), SeekOrigin.Begin);
					long offset = writer.BaseStream.Position;
                    var Hashes = new MetaIWriteList();

					for (int i = 0; i < Subfiles.Count; i++)
					{
						if (worker.CancellationPending)
						{
							e.Cancel = true;
							break;
						}

						worker.ReportProgress(i * 100 / Subfiles.Count);

                        //System.Diagnostics.Trace.WriteLine(Subfiles[i].Name);

                        if (Subfiles[i] is ppSubfile)
                        {
                            ppSubfile subfile = Subfiles[i] as ppSubfile;
                            using (MD5 md5 = MD5.Create())
                            using (MemoryStream mem = new MemoryStream())
                            using (BinaryReader reader = new BinaryReader(File.OpenRead(subfile.ppPath)))
                            {
                                reader.BaseStream.Seek(subfile.offset, SeekOrigin.Begin);
                                int bufsize = Utility.EstBufSize(subfile.size);

                                uint hash = 0;

                                if (bufsize == 0)
                                {
                                    Hashes.AddNew(Subfiles[i], hash, 0, subfile.Metadata);
                                    continue;
                                }

                                int readSteps = (int)subfile.size / bufsize;

                                byte[] buf;

                                for (int j = 0; j < readSteps; j++)
                                {
                                    buf = reader.ReadBytes(bufsize);

                                    md5.TransformBlock(buf, 0, bufsize, buf, 0);

                                    mem.WriteBytes(buf);
                                }
                                int remaining = (int)(subfile.size % bufsize);

                                buf = reader.ReadBytes(remaining);
                                md5.TransformFinalBlock(buf, 0, remaining);
                                mem.WriteBytes(buf);

                                hash = BitConverter.ToUInt32(md5.Hash, 0);

                                if (!Hashes.Any(x => x.Hash == hash))
                                {
                                    writer.Write(mem.ToArray());

                                    Hashes.AddNew(Subfiles[i], hash, (uint)(writer.BaseStream.Position - offset), subfile.Metadata);
                                }
                                else
                                {
                                    var first = Hashes.First(x => x.Hash == hash);

                                    Hashes.AddNew(Subfiles[i], hash, first.Size, first.Metadata);
                                }
                            }
                        }
                        else
                        {
                            Stream stream = ppFormat.WriteStream(writer.BaseStream);

                            using (MD5 md5 = MD5.Create())
                            using (MemoryStream mem = new MemoryStream())
                            {
                                Subfiles[i].WriteTo(mem);

                                uint hash = BitConverter.ToUInt32(md5.ComputeHash(mem.ToArray()), 0);

                                if (Hashes.Any(x => x.Hash == hash))
                                {
                                    var first = Hashes.First(x => x.Hash == hash);
                                    Hashes.AddNew(Subfiles[i], hash, first.Size, first.Metadata);
                                }
                                else
                                {
                                    mem.WriteTo(stream);
                                    object meta = ppFormat.FinishWriteTo(stream);
                                    long ppos = writer.BaseStream.Position;

                                    Hashes.AddNew(Subfiles[i], hash, (uint)(ppos - offset), meta);
                                }
                            }                            
                        }
                        offset = writer.BaseStream.Position;
                    }

					if (!e.Cancel)
					{
						writer.BaseStream.Seek(0, SeekOrigin.Begin);
                        
                        ppHeader.WriteRLEHeader(writer.BaseStream, Hashes);
					}
				}

				if (e.Cancel)
				{
					RestoreBackup(destPath, backup);
				}
				else if ((backup != null) && !keepBackup)
                {
                    File.Delete(backup);
                }

                foreach (IWriteFile iw in Subfiles)
                    if (iw is IDisposable)
                        ((IDisposable)iw).Dispose();
                
                //ReloadSubfiles();
            }
			catch (Exception ex)
			{
				RestoreBackup(destPath, backup);
                throw new Exception("PP Parser has encountered an error.", ex);
			}
		}

		void RestoreBackup(string destPath, string backup)
		{
			if (File.Exists(destPath) && File.Exists(backup))
			{
				File.Delete(destPath);

				if (backup != null)
				{
					File.Move(backup, destPath);
                    
                    ReloadSubfiles();
                }
			}
		}
	}

    public class MetaIWriteFile
    {
        public IWriteFile WriteFile;
        public uint Hash;
        public uint Size;
        public object Metadata;

        public MetaIWriteFile(IWriteFile WriteFile, uint Hash, uint Size, object Metadata)
        {
            this.WriteFile = WriteFile;
            this.Hash = Hash;
            this.Size = Size;
            this.Metadata = Metadata;
        }
    }

    public class MetaIWriteList : List<MetaIWriteFile>
    {
        public void AddNew(IWriteFile WriteFile, uint Hash, uint Size, object Metadata)
        {
            this.Add(new MetaIWriteFile(WriteFile, Hash, Size, Metadata));
        }
    }
}
