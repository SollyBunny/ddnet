#!/bin/bash

# This script should be run after
# the release tag was created
# then it will generate a changelog
# from the current release to the previous tag

declare -A known_contributors=(
	['gerdoexx@gmail.com']='gerdoe-jr'
)

git_email_to_gh_username() {
	local email="$1"
	if [[ "$email" =~ ^@[0-9]+\+(.*)@users.noreply.github.com$ ]]; then
		printf '%s' "${BASH_REMATCH[1]}"
		return 0
	fi
	known="${known_contributors["$email"]}"
	if [ "$known" != "" ]; then
		printf '%s' "$known"
		return 0
	fi
	return 1
}

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
		if ! username="$(git_email_to_gh_username "$email")"; then
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
