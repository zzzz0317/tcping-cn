/***********************************************************************
tcping.exe -- A tcp probe utility
Copyright (C) 2005-2017 Eli Fulkerson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

----------------------------------------------------------------------

Other license terms may be negotiable.  Contact the author if you would
like a copy that is licensed differently.

Contact information (as well as this program) lives at http://www.elifulkerson.com

----------------------------------------------------------------------

This application includes public domain code from the Winsock Programmer's FAQ:
  http://www.tangentsoft.net/wskfaq/
... and a big thank you to the maintainers and contributers therein.

***********************************************************************/

const char *TCPING_VERSION = "0.39";
const char *TCPING_DATE = "Dec 30 2017";

#pragma comment(lib, "Ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "tee.h"
#include "tcping.h"


using namespace std;

void usage(int argc, char* argv[]) {
    cout << "--------------------------------------------------------------" << endl;
    cout << "tcping.exe by Eli Fulkerson " << endl;
    cout << "Please see http://www.elifulkerson.com/projects/ for updates. " << endl;
	cout << "--------------------------------------------------------------" << endl;
	cout << "ZZ 汉化版 " << TCPING_VERSION << endl;
	cout << "https://home.asec01.net/" << endl;
    cout << "--------------------------------------------------------------" << endl;
    cout << endl;
    cout << "用法: " << argv[0] << " [-flags] server-address [server-port]" << endl << endl;
    cout << "用法 (完整): " << argv[0] << " [-t] [-d] [-i interval] [-n times] [-w ms] [-b n] [-r times] [-s] [-v] [-j] [-js size] [-4] [-6] [-c] [-g count] [-S source_address] [--file] [--tee filename] [-h] [-u] [--post] [--head] [--proxy-port port] [--proxy-server server] [--proxy-credentials username:password] [-f] server-address " << "[server-port]" << endl << endl;
    cout << " -t     : Ping 指定的主机，直到停止。" << endl;
	cout << "          若要停止，请键入 Ctrl+C。" << endl;
    cout << " -n 5   : 发送 5 个 ping" << endl;
    cout << " -i 5   : 每 5s ping 一次" << endl;
    cout << " -w 0.5 : 等待每次回复的超时时间(秒)。" << endl;
    cout << " -d     : 在每行中输出日期和时间" << endl;
    cout << " -b 1   : 启用 哔~ (1: 挂了的时候 , 2: 活了的时候," << endl;
    cout << "                    3: 状态变更的时候 , 4: 总是)" << endl;
    cout << " -r 5   : 每 5 次 ping 重新查找主机名" << endl;
    cout << " -s     : 在 ping 通后自动退出" <<endl;                  //[Modification 14 Apr 2011 by Michael Bray, mbray@presidio.com]
    cout << " -v     : 输出版本号然后退出" << endl;
    cout << " -j     : 包括抖动，使用默认滚动平均值"<< endl;
	cout << " -js 5  : 包括抖动，滚动平均大小（例如）5" << endl;
	cout << " --tee  : 镜像输出到 '--tee' 后指定的文件名" << endl;
	cout << " --append : 附加到 --tee 文件而不是覆盖它" << endl;
	cout << " -4     : 首选 ipv4" << endl;
	cout << " -6     : 首选 ipv6" << endl;
	cout << " -c     : 仅在更改状态下显示输出行" << endl;
	cout << " --file : 将 \"server-address\" 视为文件名，逐行循环遍历文件" << endl;
	cout << "          注意： --file 与 -j 和 -c 等选项不兼容，因为它循环遍历不同的目标" << endl;
	cout << "          可选择接受服务器端口。 例如， \"example.org 443\" 有效" << endl;
	cout << "          或者，使用 -p 在命令行强制端口为文件中的所有内容" << endl;
	cout << " -g 5   : 连续失败 5 次就放弃" << endl;
	cout << " -S _X_ : 指定源地址_X_。 源必须是客户端计算机的有效IP。" << endl;
	cout << " -p _X_ : Alternate method to specify port" << endl;
	cout << " --fqdn : 如果可用，在每一行上打印域名" << endl;
	cout << " --ansi : 使用 ANSI 颜色序列 (cygwin)" << endl;
	cout << " --color: 使用 Windows 颜色序列" << endl;
	
    cout << endl << "HTTP 选项:" << endl;
    cout << " -h     : HTTP 模式 (使用不带 http:// 的URL用于服务器地址)" << endl;
    cout << " -u     : 包括每行的目标URL" << endl;
    cout << " --post : 使用 POST 而不是 GET (可以避免缓存)" << endl;
    cout << " --head : 使用 HEAD 而不是 GET" << endl;
	cout << " --proxy-server : 指定代理服务器 " << endl;
	cout << " --proxy-port   : 指定代理服务器端口 " << endl;
	cout << " --proxy-credentials : 指定 'Proxy-Authorization: Basic' header in format username:password" << endl;
    cout << endl << "Debug 选项:" << endl;
    cout << " -f     : 强制 tcping 发送至少一个字节" << endl;
	cout << " --header : 包括带有原始参数和日期的标题。 Implied if using --tee." << endl;
	cout << " --block  : 使用 'blocking' 套接字进行连接。  这可以防止 工作并使用默认超时" << endl;
	cout << "            （在我的情况下长达20秒）。 但是，它可以检测主动拒绝的连接与超时。" << endl;
    cout << endl << "\t如果您没有传递服务器端口，则默认为 " << kDefaultServerPort << "。" << endl;

}


int main(int argc, char* argv[]) {

	

    // Do we have enough command line arguments?
    if (argc < 2) {
        usage(argc, argv);
        return 1;
    }

    int times_to_ping = 4;
    int offset = 0;  // because I don't feel like writing a whole command line parsing thing, I just want to accept an optional -t.  // well, that got out of hand quickly didn't it? -Future Eli
    double ping_interval = 1;
    int include_timestamp = 0;
    int beep_mode = 0;  // 0 is off, 1 is down, 2 is up, 3 is on change, 4 is constantly
    int ping_timeout = 2000;
	bool blocking = false;
    int relookup_interval = -1;
    int auto_exit_on_success = 0;
    int force_send_byte = 0;

    int include_url = 0;
    int use_http = 0;
    int http_cmd = 0;

    int include_jitter = 0;
    int jitter_sample_size = 0;

    int only_changes = 0;

    // for http mode
    char *serverptr;
    char *docptr = NULL;
    char server[2048];
    char document[2048];

    // for --tee
    char logfile[256];
    int use_logfile = 0;
	int show_arg_header = 0;
	bool tee_mode_append = false;

    // preferred IP version
    int ipv = 0;

	// http proxy server and port
	int proxy_port = 3128;
	char proxy_server[2048];
	proxy_server[0] = 0;

	char proxy_credentials[2048];
	int using_credentials = 0;

	// Flags for "read from filename" support
	int no_statistics = 0;  // no_statistics flag kills the statistics finale in the cases where we are reading entries from a file
	int reading_from_file = 0;  // setting this flag so we can mangle the other settings against it post parse.  For instance, it moves the meaning of -n and -t
	char urlfile[256];
	int file_times_to_loop = 1;
	bool file_loop_count_was_specific = false;   // ugh, since we are taking over the -n and -t options, but we don't want a default of 4 but we *do* want 4 if they specified 4

	int giveup_count = 0;
	int use_color = 0;

	int use_source_address = 0;
	char* src_address = "";

	int nPort = kDefaultServerPort;

	int always_print_domain = 0;

	for (int x = 0; x < argc; x++) {

		if (!strcmp(argv[x], "/?") || !strcmp(argv[x], "?") || !strcmp(argv[x], "--help") || !strcmp(argv[x], "-help")) {
			usage(argc, argv);
			return 1;
		}

		if (!strcmp(argv[x], "--proxy-port")) {
			proxy_port = atoi(argv[x + 1]);
			offset = x + 1;
		}

		if (!strcmp(argv[x], "--proxy-server")) {
			sprintf_s(proxy_server, sizeof(proxy_server), argv[x + 1]);
			offset = x + 1;
		}

		if (!strcmp(argv[x], "--proxy-credentials")) {
			sprintf_s(proxy_credentials, sizeof(proxy_credentials), argv[x + 1]);
			using_credentials = 1;
			offset = x + 1;
		}

		// force IPv4
		if (!strcmp(argv[x], "-4")) {
			ipv = 4;
			offset = x;
		}

		// force IPv6
		if (!strcmp(argv[x], "-6")) {
			ipv = 6;
			offset = x;
		}

		// ping continuously
		if (!strcmp(argv[x], "-t")) {
			times_to_ping = -1;
			file_loop_count_was_specific = true;
			offset = x;
			cout << endl << "** Pinging continuously.  Press control-c to stop **" << endl;
		}

		// Number of times to ping
		if (!strcmp(argv[x], "-n")) {
			times_to_ping = atoi(argv[x + 1]);
			file_loop_count_was_specific = true;
			offset = x + 1;
		}

		// Give up
		if (!strcmp(argv[x], "-g")) {
			giveup_count = atoi(argv[x + 1]);
			offset = x + 1;
		}

		// exit on first successful ping
		if (!strcmp(argv[x], "-s")) {
			auto_exit_on_success = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--header")) {
			show_arg_header = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--block")) {
			blocking = true;
			offset = x;
		}

		if (!strcmp(argv[x], "-p")) {
			nPort = atoi(argv[x + 1]);
			offset = x + 1;
		}

		if (!strcmp(argv[x], "--ansi")) {
			use_color = 1;
			offset = x;
		}

		if (!strcmp(argv[x], "--color")) {
			use_color = 2;
			offset = x;
		}

		if (!strcmp(argv[x], "--fqdn")) {
			always_print_domain = 1;
			offset = x;
		}

		// tee to a log file
		if (!strcmp(argv[x], "--tee")) {
			strcpy_s(logfile, sizeof(logfile), static_cast<const char*>(argv[x + 1]));
			offset = x + 1;
			use_logfile = 1;
			show_arg_header = 1;
		}

		if (!strcmp(argv[x], "--append")) {
			tee_mode_append = true;
			offset = x;
		}

		// read from a text file
		if (!strcmp(argv[x], "--file")) {
			offset = x;
			no_statistics = 1;
			reading_from_file = 1;
		}

        // http mode
        if (!strcmp(argv[x], "-h")) {
            use_http = 1;
            offset = x;
        }

        // http mode - use get
        if (!strcmp(argv[x], "--get")) {
            use_http = 1; //implied
            http_cmd = HTTP_GET;
            offset = x;
        }

        // http mode - use head
        if (!strcmp(argv[x], "--head")) {
            use_http = 1; //implied
            http_cmd = HTTP_HEAD;
            offset = x;
        }

        // http mode - use post
        if (!strcmp(argv[x], "--post")) {
            use_http = 1; //implied
            http_cmd = HTTP_POST;
            offset = x;
        }

        // include url per line
        if (!strcmp(argv[x], "-u")) {
            include_url = 1;
            offset = x;
        }

        // force send a byte
        if (!strcmp(argv[x], "-f")) {
            force_send_byte = 1;
            offset = x;
        }

        // interval between pings
        if (!strcmp(argv[x], "-i")) {
            ping_interval = atof(argv[x+1]);
            offset = x+1;
        }

        // wait for response
        if (!strcmp(argv[x], "-w")) {
			ping_timeout = (int)(1000 * atof(argv[x + 1]));
            offset = x+1;
        }

		// source address
		if (!strcmp(argv[x], "-S")) {
			src_address = argv[x + 1];
			use_source_address = 1;
			offset = x + 1;
		}

        // optional datetimestamp output
        if (!strcmp(argv[x], "-d")) {
            include_timestamp = 1;
            offset = x;
        }

        // optional jitter output
        if (!strcmp(argv[x], "-j")) {
            include_jitter = 1;
            offset = x;
		}
     
		// optional jitter output (sample size)
		if (!strcmp(argv[x], "-js")) {
            include_jitter = 1;
            offset = x;

            // obnoxious special casing if they actually specify the default 0
            if (!strcmp(argv[x+1], "0")) {
                jitter_sample_size = 0;
                offset = x+1;
            } else {
                if (atoi(argv[x+1]) == 0) {
                    offset = x;
                } else {
                    jitter_sample_size = atoi(argv[x+1]);
                    offset = x+1;
                }
            }
            //			cout << "offset coming out "<< offset << endl;
        }

        // optional hostname re-lookup
        if (!strcmp(argv[x], "-r")) {
            relookup_interval = atoi(argv[x+1]);
            offset = x+1;
        }
		
		 // optional output minimization
        if (!strcmp(argv[x], "-c")) {
            only_changes = 1;
            offset = x;
			cout << endl << "** Only displaying output for state changes. **" << endl;
        }

        // optional beepage
        if (!strcmp (argv[x], "-b")) {
            beep_mode = atoi(argv[x+1]);
            offset = x+1;
            switch (beep_mode) {
            case 0:
                break;
            case 1:
                cout << endl << "** Beeping on \"down\" - (two beeps) **" << endl;
                break;
            case 2:
                cout << endl << "** Beeping on \"up\"  - (one beep) **" << endl;
                break;
            case 3:
                cout << endl << "** Beeping on \"change\" - (one beep up, two beeps down) **" << endl;
                break;
            case 4:
                cout << endl << "** Beeping constantly - (one beep up, two beeps down) **" << endl;
                break;
            }

        }

        // dump version and quit
        if (!strcmp(argv[x], "-v") || !strcmp(argv[x], "--version") ) {
            //cout << "tcping.exe 0.30 Nov 13 2015" << endl;
			cout << "tcping.exe " << TCPING_VERSION << " " << TCPING_DATE << endl;
            cout << "compiled: " << __DATE__ << " " << __TIME__ <<  endl;
            cout << endl;
            cout << "tcping.exe by Eli Fulkerson " << endl;
            cout << "Please see http://www.elifulkerson.com/projects/ for updates. " << endl;
            cout << endl;
            cout << "-s option contributed 14 Apr 2011 by Michael Bray, mbray@presidio.com" << endl;
			cout << "includes base64.cpp Copyright (C) 2004-2008 Ren?Nyffenegger" << endl;
            return 1;
        }
	}

	// open our logfile, if applicable
	tee out;
	if (use_logfile == 1 && logfile != NULL) {
		if (tee_mode_append == true) {
			out.OpenAppend(logfile);
		} else {
			out.Open(logfile);
		}
	}



	if (show_arg_header == 1) {
		out.p("-----------------------------------------------------------------\n");
		// print out the args
		out.p("args: ");
		for (int x = 0; x < argc; x++) {
			out.pf("%s ", argv[x]);
		}
		out.p("\n");


		// and the date

		time_t rawtime;
		struct tm  timeinfo;
		char dateStr[11];
		char timeStr[9];

		errno_t err;

		_strtime_s(timeStr, sizeof(timeStr));

		time(&rawtime);

		err = localtime_s(&timeinfo, &rawtime);
		strftime(dateStr, 11, "%Y:%m:%d", &timeinfo);
		out.pf("date: %s %s\n", dateStr, timeStr);

		// and the attrib
		out.pf("tcping.exe v%s: http://www.elifulkerson.com/projects/tcping.php\n", TCPING_VERSION);
		out.p("-----------------------------------------------------------------\n");

	}





	// Get host and (optionally) port from the command line

	char* pcHost = "";
	//char pcHost[2048] = "";
	
    if (argc >= 2 + offset) {
		if (!reading_from_file) {
			pcHost = argv[1 + offset];
		}
		else {
			strcpy_s(urlfile, sizeof(urlfile), static_cast<const char*>(argv[offset + 1]));
		}


    } else {
			cout << "Check the last flag before server-address.  Did you specify a flag and forget its argument?" << endl;
			return 1;
    }

    
	// allow the -p option to win if we set it
    if (argc >= 3 + offset && nPort == kDefaultServerPort) {
        nPort = atoi(argv[2 + offset]);
    }

    // Do a little sanity checking because we're anal.
    int nNumArgsIgnored = (argc - 3 - offset);
    if (nNumArgsIgnored > 0) {
        cout << nNumArgsIgnored << " extra argument" << (nNumArgsIgnored == 1 ? "" : "s") << " ignored.  FYI." << endl;
    }

    if (use_http == 1 && reading_from_file == 0) {   //added reading from file because if we are doing multiple http this message is just spam.
        serverptr = strchr(pcHost, ':');
        if (serverptr != NULL) {
            ++serverptr;
            ++serverptr;
            ++serverptr;
        } else {
            serverptr = pcHost;
        }

        docptr = strchr(serverptr, '/');
        if (docptr != NULL) {
            *docptr = '\0';
            ++docptr;

			strcpy_s(server, sizeof(server), static_cast<const char*>(serverptr));
			strcpy_s(document, sizeof(document), static_cast<const char*>(docptr));
        } else {
			strcpy_s(server, sizeof(server), static_cast<const char*>(serverptr));
            document[0] = '\0';
        }

		out.pf("\n** Requesting %s from %s:\n", document, server);
		out.p("(for various reasons, kbit/s is an approximation)\n");
    }

    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // Start Winsock up
    WSAData wsaData;
    int nCode;
    if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0) {
        cout << "WSAStartup() returned error code " << nCode << "." << endl;
        return 255;
    }

    // Call the main example routine.
	int retval;

	out.p("\n");

	if (!reading_from_file) {
		retval = DoWinsock_Single(pcHost, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, out, use_source_address, src_address, blocking, always_print_domain, use_color);
	}
	else {
		if (file_loop_count_was_specific) {
			file_times_to_loop = times_to_ping;
		}
		times_to_ping = 1;
		retval = DoWinsock_Multi(pcHost, nPort, times_to_ping, ping_interval, include_timestamp, beep_mode, ping_timeout, relookup_interval, auto_exit_on_success, force_send_byte, include_url, use_http, docptr, http_cmd, include_jitter, jitter_sample_size, logfile, use_logfile, ipv, proxy_server, proxy_port, using_credentials, proxy_credentials, only_changes, no_statistics, giveup_count, file_times_to_loop, urlfile, out, use_source_address, src_address, blocking, always_print_domain, use_color);
	}

    // Shut Winsock back down and take off.
    WSACleanup();
    return retval;
}

