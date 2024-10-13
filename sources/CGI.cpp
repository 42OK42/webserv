/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggiertzu <ggiertzu@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:40:43 by ggiertzu          #+#    #+#             */
/*   Updated: 2024/10/14 00:53:47 by ggiertzu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>

char **get_env(const std::map<std::string, std::string> &headers) {
	char	**envp;
	int		num_headers;
	std::string env_entry;

	num_headers = headers.size();
	envp = new char*[num_headers + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		env_entry = it->first + "=" + it->second;
		envp[i] = new char[env_entry.length() + 1];
		std::strcpy(envp[i], env_entry.c_str());
		i++;
	}
	envp[i] = 0;
return envp;
}


int	gen_cgi_res(const HttpRequest& req, HttpResponse& res) {
	pid_t		pid;  // Fork the process
	char		**envp;
	int			pipein[2], pipeout[2];
	int			wstatus;
	int			nbytes;
	char		buffer[1024];
	std::string	cgi_path;
	std::ostringstream oss;
	pipe(pipeout);

	cgi_path = "cgi-bin/contact_form.py";
	if ((pid = fork()) == 0) {
		envp = get_env(req.getHeaders());
		pipe(pipein);
		dup2(pipeout[1], STDOUT_FILENO);
		close(pipeout[0]);
		// do we need poll() here?
		write(pipein[1], req.getBody().c_str(), req.getBody().size());
		close(pipein[1]);
		dup2(pipein[0], STDIN_FILENO);
		char *argv[] = {const_cast <char *> (cgi_path.c_str()), NULL};
		execve(cgi_path.c_str(), argv, envp);
		std::cout << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	} else if (pid > 0) {  // Parent process
		close(pipeout[1]);
		while ((nbytes = read(pipeout[0], buffer, 1024)) > 0) {
			oss.write(buffer, nbytes);
		}
		close(pipeout[0]);
		waitpid(pid, &wstatus, 0);
	} else {
		std::cout << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << oss.str() << std::endl;

	// improve python script parsing here
	res.setStatusCode(200);
	res.setBody(oss.str().substr(oss.str().find("<html>")));
	res.setHeader("Content-Type", "text/html");
	return 0;
}
