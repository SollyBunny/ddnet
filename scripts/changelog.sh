#!/bin/bash

set -u

# This script should be run after
# the release tag was created
# then it will generate a changelog
# from the current release to the previous tag

declare -A known_contributors=(
	['gerdoexx@gmail.com']='gerdoe-jr'
	['84133700+ByFox213@users.noreply.github.com']='ByFox213'
	['byfox213@gmail.com']='ByFox213'
)

# hack to avoid triggering 'set -u' unbound var error
# when looking up unknown emails
is_contributor_known() {
	if [ "${known_contributors["$email"]+check_key_no_unset}" ]; then
		return 0
	fi
	return 1
}

git_email_to_gh_username() {
	local email="$1"
	if [[ "$email" =~ ^@[0-9]+\+(.*)@users.noreply.github.com$ ]]; then
		printf '%s' "${BASH_REMATCH[1]}"
		return 0
	fi
	if ! is_contributor_known "$email"; then
		return 1
	fi
	known="${known_contributors["$email"]}"
	if [ "$known" != "" ]; then
		printf '%s' "$known"
		return 0
	fi
	return 1
}

git_commit_sha_to_gh_username() {
	local commit_sha="$1"
	local username
	if username="$(gh api "/repos/ddnet-insta/ddnet-insta/commits/$commit_sha" --jq '.author.login')"; then
		printf -- '%s' "$username"
		return 0
	fi
	exit 1
}

gh_username_by_email_or_commit_sha() {
	local email="$1"
	local commit_sha="$2"
	local username

	# try email cache first
	if username="$(git_email_to_gh_username "$email")"; then
		printf -- '%s' "$username"
		return 0
	fi

	# use github api as fallback
	if username="$(git_commit_sha_to_gh_username "$commit_sha")"; then
		printf -- '%s' "$username"

		# store in cache
		known_contributors["$email"]="$username"
		return 0
	fi

	exit 1
}

check_gh_cli() {
	if [ ! -x "$(command -v gh)" ]; then
		echo "Error: command 'gh' not found. You need to install the github cli"
		exit 1
	fi
	if ! gh auth status > /dev/null 2>&1; then
		echo "Error: you need to login with github cli: gh auth login"
		exit 1
	fi
}

check_gh_cli

current_tag="$(git --no-pager tag --sort=-creatordate | head -n1)"
# the [0-9][0-9] exclude should match all ddnet tags like 18.7
# so we only get ddnet-insta releases here
previous_tag="$(git describe --tags --abbrev=0 "$current_tag"^ --exclude '[0-9][0-9].*')"

printf "# Changelog\n\n"

tag_date=$(git log -1 --pretty=format:'%ad' --date=short "$current_tag")
printf "## %s (%s)\n\n" "$current_tag" "$tag_date"
while read -r commit; do
	[ "$(git branch ddnet --contains "$commit")" == "" ] || continue

	line="$(git \
		show "$commit" \
		--no-patch \
		--pretty=format:'* %s [View](https://github.com/ddnet-insta/ddnet-insta/commit/%H)')"
	email="$(git \
		show "$commit" \
		--no-patch \
		--pretty=format:'%ae')"
	if [[ "${email,,}" != "chillerdragon@gmail.com" ]]; then
		if ! username="$(gh_username_by_email_or_commit_sha "$email" "$commit")"; then
			username="$(git \
				show "$commit" \
				--no-patch \
				--pretty=format:'%an')"
		fi
		line="$line @$username"
	fi
	[[ "$line" = '* Merge branch '* ]] && continue
	[[ "$line" = '* Merge pull request #'* ]] && continue
	[[ "$line" = "* Merge remote-tracking branch '"* ]] && continue
	[[ "$line" = "* merge: "* ]] && continue

	printf '%s\n' "$line"
done < <(git --no-pager log "${previous_tag}...$current_tag" --pretty=format:'%H')
printf "\n\n"
